#include"TestType.hlsli"

Texture2D<float4> tex : register(t0);
SamplerState smp : register(s0);

// シーン共通（ビュー・プロジェクション）
cbuffer SceneConstant : register(b0)
{
    matrix view;
    matrix proj;
};

cbuffer MaterialConstant : register(b2)
{
    float4 baseColor;
    float metallic;
    float roughness;
    float reflectance;
    float _pad0;
};

float3 FresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0f - F0) * pow(1.0f - cosTheta, 5.0f);
}

float4 PS(TestType input) : SV_TARGET
{
    float3 albedo = tex.Sample(smp, input.uv).rgb * baseColor.rgb;

    // view 空間へ変換（カメラは view 空間原点）
    float3 N = normalize(mul(view, float4(normalize(input.normal.xyz), 0.0f)).xyz);
    float3 viewPos = mul(view, float4(input.worldPos, 1.0f)).xyz;
    float3 V = normalize(-viewPos);

    // 簡易ディレクショナルライト（view 空間）
    float3 L = normalize(float3(1.0f, 1.0f, 1.0f));
    float3 H = normalize(L + V);

    float NoL = saturate(dot(N, L));
    float NoV = saturate(dot(N, V));
    float NoH = saturate(dot(N, H));

    float m = saturate(metallic);
    float r = saturate(roughness);
    float refl = saturate(reflectance);

    // Fresnel: 非金属は 0.04、金属はアルベド色寄り
    float3 F0 = lerp(float3(0.04f, 0.04f, 0.04f), albedo * refl, m);
    float3 F = FresnelSchlick(NoV, F0);

    // Diffuse（metallic が高いほど抑える）
    float diffuseStrength = (1.0f - m) * (1.0f - 0.5f * refl);
    float3 diffuse = albedo * NoL * diffuseStrength;

    // Specular（roughnessで鋭さ、reflectanceで強さ）
    float gloss = 1.0f - r;
    float specPower = lerp(8.0f, 128.0f, gloss);
    float specStrength = lerp(0.04f, 1.0f, refl);
    float3 specular = F * pow(NoH, specPower) * specStrength * NoL;

    float3 ambient = albedo * 0.08f;
    float3 color = ambient + diffuse + specular;

    return float4(color, 1.0f);
    //return float4(input.uv.xy, 1, 1);
}