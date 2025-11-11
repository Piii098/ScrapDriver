RaytracingAccelerationStructure SceneBVH : register(t0);
Texture2D<float4> DiffuseTexture : register(t1);
SamplerState DiffuseSampler : register(s0);
RWTexture2D<float4> RenderTarget : register(u0);

// カメラデータ
cbuffer CameraData : register(b0)
{
    float3 CameraPosition;
    float CameraFOV;
    float3 CameraForward;
    float CameraAspectRatio;
    float3 CameraRight;
    float _padding1;
    float3 CameraUp;
    float _padding2;
}

// マテリアルデータ
cbuffer MaterialData : register(b1)
{
    float reflectanceCoef;      
    float diffuseCoef;          
    float specularCoef;         
    float specularPower;        
    float3 lightPosition;       
    float padding3;
    float4 lightColor;          
}

#define MAX_RAY_RECURSION_DEPTH 5  // 2→3に変更（5以上は推奨しない）

struct RayPayload
{
    float4 color;
    uint recursionDepth;
};

struct ShadowPayload
{
    bool hit;
};

struct VertexAttributes
{
    float2 uv;
};

float3 FresnelReflectanceSchlick(float3 incidentDir, float3 normal, float3 f0)
{
    float cosTheta = saturate(dot(-incidentDir, normal));
    return f0 + (1.0 - f0) * pow(1.0 - cosTheta, 5.0);
}

float4 TraceReflectionRay(float3 origin, float3 direction, uint currentDepth)
{
    if(currentDepth >= MAX_RAY_RECURSION_DEPTH)
    {
        return float4(0, 0, 0, 1); // 最大深度に達した場合、黒を返す
    }

    RayDesc reflectionRay;                  // 反射レイの定義
    reflectionRay.Origin = origin;          // レイの始点
    reflectionRay.Direction = direction;    // レイの方向
    reflectionRay.TMin = 0.001f;            // 最小距離    
    reflectionRay.TMax = 10000.0f;          // 最大距離 

    RayPayload reflectionPayload;           // 反射レイのペイロード
    reflectionPayload.color = float4(0, 0, 0, 1); // 初期色
    reflectionPayload.recursionDepth = currentDepth + 1; // 再帰深度を更新

    TraceRay(SceneBVH, RAY_FLAG_NONE, 0xFF, 0, 0, 0, reflectionRay, reflectionPayload);

    return reflectionPayload.color;
}

bool TraceShadowray(float3 origin, float3 lightDir, float lightDistance)
{
    RayDesc shadowRay;                       // シャドウレイの定義
    shadowRay.Origin = origin;               // レイの始点
    shadowRay.Direction = lightDir;          // レイの方向
    shadowRay.TMin = 0.001f;                 // 最小距離
    shadowRay.TMax = lightDistance - 0.001f; // ライトまでの距離より少し手前まで

    ShadowPayload shadowPayload;             // シャドウペイロード
    shadowPayload.hit = false;               // 初期状態はヒットしていない

    TraceRay(SceneBVH, RAY_FLAG_NONE, 0xFF, 0, 0, 0, shadowRay, shadowPayload);

    return shadowPayload.hit;
}

[shader("raygeneration")]
void MyRaygenShader()
{
    uint2 launchIndex = DispatchRaysIndex().xy;
    uint2 launchDim = DispatchRaysDimensions().xy;
    
    // NDC座標に変換
    float2 ndc = (float2(launchIndex) + 0.5f) / float2(launchDim) * 2.0f - 1.0f;
    ndc.y = -ndc.y;
    
    // カメラのFOVとアスペクト比を考慮
    float tanHalfFOV = tan(CameraFOV * 0.5f);
    ndc.x *= CameraAspectRatio * tanHalfFOV;
    ndc.y *= tanHalfFOV;
    
    // カメラの向きに基づいてレイの方向を計算
    float3 rayDir = normalize(CameraForward + ndc.x * CameraRight + ndc.y * CameraUp);
    
    // レイの生成
    RayDesc ray;
    ray.Origin = CameraPosition;
    ray.Direction = rayDir;
    ray.TMin = 0.001f;
    ray.TMax = 10000.0f;
    
    RayPayload payload;
    payload.color = float4(1.0f, 0.0f, 1.0f, 1.0f); // マゼンタ（デフォルト色）
    payload.recursionDepth = 0;
    
    TraceRay(SceneBVH, RAY_FLAG_NONE, 0xFF, 0, 0, 0, ray, payload);
    
    RenderTarget[launchIndex] = payload.color;
}

[shader("closesthit")]
void MyClosestHitShader(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attr)
{
    float3 hitPosition = WorldRayOrigin() + WorldRayDirection() * RayTCurrent();
    
    float2 uv;
    float3 normal;
    float uvScale = 2.0f;
    
    // メッシュの種類を判定
    if (abs(hitPosition.y + 1.0f) < 1.0f) {
        // 床
        uv = float2(
            (hitPosition.x + 50.0f) / 100.0f * uvScale,
            (hitPosition.z + 50.0f) / 100.0f * uvScale
        );
        normal = float3(0, 1, 0);
    }
    else if (abs(hitPosition.x + 25.0f) < 1.0f && hitPosition.y > 0.0f) {
        // X = -25の壁
        uv = float2(
            (hitPosition.z + 25.0f) / 50.0f * uvScale,
            hitPosition.y / 50.0f * uvScale
        );
        normal = float3(1, 0, 0);
    }
    else if (abs(hitPosition.y - 10.0f) < 1.0f) {
        // Y = 10の上部メッシュ
        uv = float2(
            (hitPosition.x + 0.5f) / 1.0f * uvScale,
            (hitPosition.z + 4.5f) / 1.0f * uvScale
        );
        normal = float3(0, 0, -1);
    }
    else {
        uv = float2(0.5f, 0.5f);
        normal = float3(0, 1, 0);
    }
    
    uv = frac(uv);
    
    // 裏面判定を強化
    float facingRatio = dot(normal, -WorldRayDirection());
    bool isFrontFace = facingRatio > 0.0f;
    
    // 裏面の場合は法線を反転
    if (!isFrontFace) {
        normal = -normal;
        facingRatio = -facingRatio;
    }
    
    // 裏面の場合は早期リターン（オプション：裏面も描画する場合はコメントアウト）
    if (!isFrontFace) {
        // 裏面は暗い色で表示
        payload.color = float4(0.1f, 0.1f, 0.1f, 1.0f);
        return;
    }
    
    float4 texColor = DiffuseTexture.SampleLevel(DiffuseSampler, uv, 0);
    
    float3 lightDir = normalize(lightPosition - hitPosition);
    float NdotL = max(0.0f, dot(normal, lightDir));
    
    float3 diffuse = texColor.rgb * lightColor.rgb * NdotL * diffuseCoef;
    float3 ambient = texColor.rgb * 0.2f;
    
    float3 viewDir = normalize(CameraPosition - hitPosition);
    float3 halfDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfDir), 0.0f), specularPower);
    float3 specular = lightColor.rgb * spec * specularCoef;
    
    // シャドウレイによる影の計算
    float shadowFactor = 1.0f;
    float lightDistance = length(lightPosition - hitPosition);
    
    // 光源方向が正しいかチェック
    if (NdotL > 0.0f) {
        if (TraceShadowray(hitPosition + normal * 0.01f, lightDir, lightDistance))
        {
            shadowFactor = 0.3f;
        }
    }
    else {
        // 光源が反対側にある場合は完全に影
        shadowFactor = 0.3f;
    }
    
    diffuse *= shadowFactor;
    specular *= shadowFactor;
    
    float3 localColor = ambient + diffuse + specular;
    
    // フレネル効果を適用
    float3 reflectedColor = float3(0, 0, 0);
    
    if (isFrontFace && 
        payload.recursionDepth < MAX_RAY_RECURSION_DEPTH && 
        reflectanceCoef > 0.1f)
    {
        float3 reflectDir = reflect(WorldRayDirection(), normal);
        
        if (dot(reflectDir, normal) > 0.0f) {
            float4 reflectionResult = TraceReflectionRay(
                hitPosition + normal * 0.01f,
                reflectDir,
                payload.recursionDepth
            );
            reflectedColor = reflectionResult.rgb;
        }
    }
    
    // フレネル係数を計算
    float3 f0 = float3(reflectanceCoef, reflectanceCoef, reflectanceCoef);
    float3 fresnel = FresnelReflectanceSchlick(WorldRayDirection(), normal, f0);
    
    // エネルギー保存則を考慮した合成
    // 反射が増えれば拡散が減る
    float3 kD = 1.0f - fresnel; // 拡散反射の係数
    
    float3 finalColor = kD * localColor + fresnel * reflectedColor;
    
    payload.color = float4(finalColor, 1.0f);
}

[shader("miss")]
void MyMissShader(inout RayPayload payload)
{
    // 背景色（青空）
    payload.color = float4(0.4f, 0.6f, 0.9f, 1.0f);
}

[shader("closesthit")]
void ShadowClosestHitShader(inout ShadowPayload payload, in BuiltInTriangleIntersectionAttributes attr)
{
    payload.hit = true;
}

[shader("miss")]
void ShadowMissShader(inout ShadowPayload payload)
{
    payload.hit = false;
}