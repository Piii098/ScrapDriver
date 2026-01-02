#include"TestType.hlsli"

Texture2D<float4> tex : register(t0);
Texture2D<float> shadowMap : register(t1);
SamplerState smp : register(s0);
SamplerComparisonState shadowSampler : register(s1);

cbuffer MaterialConstant : register(b2)
{
    float4 baseColor;
};

// Light data structure
struct LightData {
    float3 position;
    uint type;  // 0=Directional, 1=Point, 2=Spot
    float3 direction;
    float range;
    float4 color;
    float intensity;
    float innerAngle;
    float outerAngle;
    float _padding;
};

cbuffer LightsConstant : register(b3)
{
    uint lightCount;
    float3 _padding0;
    LightData lights[8];
    matrix lightViewProj; // Light space view-projection matrix for shadows
};

// PCF shadow sampling
float CalcShadowFactor(float4 lightSpacePos)
{
    // Perspective divide
    float3 projCoords = lightSpacePos.xyz / lightSpacePos.w;
    
    // Transform to [0,1] range for texture sampling
    projCoords.x = projCoords.x * 0.5f + 0.5f;
    projCoords.y = projCoords.y * -0.5f + 0.5f;
    
    // Outside shadow map bounds - no shadow
    if (projCoords.x < 0.0f || projCoords.x > 1.0f ||
        projCoords.y < 0.0f || projCoords.y > 1.0f ||
        projCoords.z < 0.0f || projCoords.z > 1.0f)
    {
        return 1.0f;
    }
    
    float currentDepth = projCoords.z;
    
    // PCF (Percentage Closer Filtering) for soft shadows
    float shadow = 0.0f;
    float2 texelSize = 1.0f / float2(2048.0f, 2048.0f); // Shadow map size
    
    const int pcfRadius = 1;
    for (int x = -pcfRadius; x <= pcfRadius; ++x)
    {
        for (int y = -pcfRadius; y <= pcfRadius; ++y)
        {
            float2 offset = float2(x, y) * texelSize;
            float pcfDepth = shadowMap.Sample(smp, projCoords.xy + offset).r;
            shadow += (currentDepth - 0.005f) > pcfDepth ? 0.0f : 1.0f;
        }
    }
    
    int pcfSize = (pcfRadius * 2 + 1);
    shadow /= float(pcfSize * pcfSize);
    
    return shadow;
}

float4 PS(TestType input) : SV_TARGET
{
    // Calculate light space position for shadow mapping
    float4 worldPos = float4(input.worldPos, 1.0f);
    float4 lightSpacePos = mul(lightViewProj, worldPos);
    
    // Sample textures
    float4 texColor = tex.Sample(smp, input.uv);
    
    // Calculate shadow factor
    float shadowFactor = CalcShadowFactor(lightSpacePos);
    
    // Simple lighting calculation
    float3 normal = normalize(input.normal.xyz);
    
    // Ambient light
    float3 ambient = 0.2f * texColor.rgb * baseColor.rgb;
    
    // Directional light (simplified)
    float3 lightDir = float3(0, 0, 0);
    float3 lightColor = float3(1, 1, 1);
    float lightIntensity = 1.0f;
    
    if (lightCount > 0)
    {
        LightData light = lights[0];
        if (light.type == 0) // Directional
        {
            lightDir = normalize(-light.direction);
        }
        else if (light.type == 1 || light.type == 2) // Point or Spot
        {
            lightDir = normalize(light.position - input.worldPos);
        }
        lightColor = light.color.rgb;
        lightIntensity = light.intensity;
    }
    else
    {
        // Default light
        lightDir = normalize(float3(1, 1, 1));
    }
    
    float diffuse = max(dot(normal, lightDir), 0.0f);
    float3 diffuseColor = diffuse * lightColor * lightIntensity * texColor.rgb * baseColor.rgb;
    
    // Apply shadow factor to direct lighting only
    float3 finalColor = ambient + diffuseColor * shadowFactor;
    
    return float4(finalColor, 1.0f);
}