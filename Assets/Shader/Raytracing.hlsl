RaytracingAccelerationStructure SceneBVH : register(t0);
Texture2D<float4> DiffuseTexture : register(t1);
SamplerState DiffuseSampler : register(s0);
RWTexture2D<float4> RenderTarget : register(u0);

// メッシュごとのマテリアルデータ配列
struct PerMeshMaterialData {
    float4 baseColor;          // 基本色（RGBA）
    float metallic;            // メタリック（0.0-1.0）
    float roughness;           // ラフネス（0.0-1.0）
    float reflectance;         // 反射率（0.0-1.0）
    uint materialType;         // マテリアルタイプ（0=Diffuse, 1=Metal, 2=Glass）
    float2 uvScale;            // UVスケール
    float2 uvOffset;           // UVオフセット
    float3 uvAxis1;            // UV計算用の軸1
    float uvRange1;            // 軸1の範囲
    float3 uvAxis2;            // UV計算用の軸2
    float uvRange2;            // 軸2の範囲
    uint textureIndex;         // テクスチャインデックス
    float3 padding;            // アラインメント用
};

StructuredBuffer<PerMeshMaterialData> MeshMaterials : register(t2);

// テクスチャ配列（スマートな方法）
Texture2D<float4> TextureArray[16] : register(t3);

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
    float ambientIntensity;     
    float4 lightColor;          
}

struct RayPayload
{
    float4 color;
};

struct VertexAttributes
{
    float2 uv;
};

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
    payload.color = float4(0.4f, 0.6f, 0.9f, 1.0f); // デフォルトは空の色
    
    TraceRay(SceneBVH, RAY_FLAG_NONE, 0xFF, 0, 0, 0, ray, payload);
    
    RenderTarget[launchIndex] = payload.color;
}

[shader("closesthit")]
void MyClosestHitShader(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attr)
{
    // ヒット座標
    float3 hitPosition = WorldRayOrigin() + WorldRayDirection() * RayTCurrent();

    // InstanceIDからマテリアルを取得
    uint instanceID = InstanceID();
    PerMeshMaterialData meshMaterial = MeshMaterials[instanceID];

    // 安全対策（ゼロ除算を避ける）
    float eps = 1e-6f;
    float r1 = max(meshMaterial.uvRange1, eps);
    float r2 = max(meshMaterial.uvRange2, eps);

    // データ駆動UV計算
    float uCoord = dot(hitPosition, meshMaterial.uvAxis1) / r1;
    float vCoord = dot(hitPosition, meshMaterial.uvAxis2) / r2;
    float2 uv = float2(uCoord, vCoord) + meshMaterial.uvOffset;
    uv = frac(uv * max(meshMaterial.uvScale, float2(eps, eps)));

    // テクスチャサンプリング
    float3 tex = float3(1.0f, 1.0f, 1.0f); // デフォルトは白
    if (meshMaterial.textureIndex != 0xFFFFFFFF && meshMaterial.textureIndex < 16) 
    {
        tex = TextureArray[meshMaterial.textureIndex].SampleLevel(DiffuseSampler, uv, 0).rgb;
    }

    // 法線の計算（ダミーとして上向き）
    float3 normal = float3(0, 1, 0);
    
    float3 edge1 = v1 - v0;
    float3 edge2 = v2 - v0;
    normal = normalize(cross(edge1, edge2));
    
    // ライティング計算
    float3 lightDir = normalize(lightPosition - hitPosition);
    float3 viewDir = normalize(CameraPosition - hitPosition);
    float3 halfVec = normalize(lightDir + viewDir);

    // 拡散反射(Lambert)
    float diffuse = max(dot(normal, lightDir), 0.0f) * diffuseCoef;

    // 鏡面反射(Blinn-Phong)
    float specular = pow(max(dot(normal, halfVec), 0.0f), specularPower) * specularCoef;

    // 環境光
    float3 ambient = tex * ambientIntensity;

    // 反射レイを飛ばす
    float3 reflectColor = float3(0, 0, 0);
    if (reflectanceCoef > 0.001f)
    {
        float3 reflectDir = reflect(WorldRayDirection(), normal);
        RayDesc reflectRay;
        reflectRay.Origin = hitPosition + normal * 0.001f;
        reflectRay.Direction = reflectDir;
        reflectRay.TMin = 0.001f;
        reflectRay.TMax = 10000.0f;

        RayPayload reflectPayload;
        reflectPayload.color = float4(0, 0, 0, 1);
        TraceRay(SceneBVH, RAY_FLAG_NONE, 0xFF, 0, 0, 0, reflectRay, reflectPayload);
        reflectColor = reflectPayload.color.rgb;
    }

    // 最終色の合成
    float3 finalColor = ambient + 
                        (diffuse * tex + specular) * lightColor.rgb +
                        reflectColor * reflectanceCoef;

    payload.color = float4(finalColor, 1.0f);
}

[shader("miss")]
void MyMissShader(inout RayPayload payload)
{
    // 背景色（青空）
    payload.color = float4(0.4f, 0.6f, 0.9f, 1.0f);
}