RaytracingAccelerationStructure SceneBVH : register(t0);
Texture2D<float4> DiffuseTexture : register(t1);
SamplerState DiffuseSampler : register(s0);
RWTexture2D<float4> RenderTarget : register(u0);

#define MAX_VERTEX_BUFFERS 256 // Must match kMaxRaytracingVertexBuffers in C++
StructuredBuffer<float3> VertexBuffers[MAX_VERTEX_BUFFERS] : register(t19);
// 頂点法線（StructuredBuffer<float3>）配列
// VertexBuffers が t19-t274 を占有するため、その後ろの t275 から開始
StructuredBuffer<float3> NormalBuffers[MAX_VERTEX_BUFFERS] : register(t275);

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

#define MAX_BOUNCES 2 // 反射回数（0=反射なし, 1=1回反射...）

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

// ライトデータ（シーン共通）
// 係数（拡散/鏡面/反射の強さなど）は PerMeshMaterialData 側で制御する。
cbuffer LightingData : register(b1)
{
    float3 lightPosition;
    float ambientIntensity;
    float4 lightColor;
}

struct RayPayload
{
    float4 color;
    uint depth;
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
    payload.color = float4(0.4f, 0.6f, 0.9f, 1.0f);
    payload.depth = 0;

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

    // 頂点データ取得（非インデックス三角形想定）
    if (instanceID >= MAX_VERTEX_BUFFERS)
    {
        payload.color = float4(1, 0, 1, 1); // 安全対策（想定外のID）
        return;
    }

    uint primitiveIndex = PrimitiveIndex();
    uint baseVertex = primitiveIndex * 3;

    float3x4 objectToWorld = ObjectToWorld3x4();
    float3 p0 = mul(objectToWorld, float4(VertexBuffers[instanceID][baseVertex + 0], 1.0f));
    float3 p1 = mul(objectToWorld, float4(VertexBuffers[instanceID][baseVertex + 1], 1.0f));
    float3 p2 = mul(objectToWorld, float4(VertexBuffers[instanceID][baseVertex + 2], 1.0f));

    // 面法線（ワールド空間）
    float3 faceNormal = cross(p1 - p0, p2 - p0);
    float faceNormalLenSq = dot(faceNormal, faceNormal);
    faceNormal = (faceNormalLenSq > 1e-8f) ? normalize(faceNormal) : float3(0, 1, 0);

    // スムース法線：頂点法線をバリセントリック補間（オブジェクト空間 -> ワールド空間）
    float2 bary = attr.barycentrics;
    float b0 = 1.0f - bary.x - bary.y;
    float b1 = bary.x;
    float b2 = bary.y;

    float3 n0 = NormalBuffers[instanceID][baseVertex + 0];
    float3 n1 = NormalBuffers[instanceID][baseVertex + 1];
    float3 n2 = NormalBuffers[instanceID][baseVertex + 2];

    float3 objectNormal = n0 * b0 + n1 * b1 + n2 * b2;
    float objectNormalLenSq = dot(objectNormal, objectNormal);
    objectNormal = (objectNormalLenSq > 1e-8f) ? normalize(objectNormal) : float3(0, 1, 0);

    float3x3 worldToObject = (float3x3)WorldToObject3x4();
    float3 smoothNormal = mul(transpose(worldToObject), objectNormal);
    float smoothNormalLenSq = dot(smoothNormal, smoothNormal);
    smoothNormal = (smoothNormalLenSq > 1e-8f) ? normalize(smoothNormal) : faceNormal;

    // 非対応メッシュ/法線が無効な場合に備えてフォールバック
    float3 normal = (smoothNormalLenSq > 1e-8f) ? smoothNormal : faceNormal;

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

    // メッシュマテリアルのパラメータを使用
    float3 baseColor = meshMaterial.baseColor.rgb;
    float metallic = saturate(meshMaterial.metallic);
    float roughness = saturate(meshMaterial.roughness);
    float reflectance = saturate(meshMaterial.reflectance);

    float3 albedo = baseColor * tex;

    float3 lightDir = normalize(lightPosition - hitPosition);
    float3 viewDir  = normalize(CameraPosition - hitPosition);
    float3 halfVec  = normalize(lightDir + viewDir);

    // --- Fresnel（Schlick 近似）---
    float NoV = saturate(dot(normal, viewDir));          // 法線と視線の角度
    // 金属度と反射率から基礎反射率 F0 を決定（0.04 は非金属の典型値）
    float3 F0 = lerp(float3(0.04f, 0.04f, 0.04f), albedo * reflectance, metallic);
    float3 fresnel = F0 + (1.0f - F0) * pow(1.0f - NoV, 5.0f);


    // 拡散反射(Lambert) — metallic/reflectance が高いほど減衰
    float diffuseStrength = (1.0f - metallic) * (1.0f - 0.5f * reflectance);
    diffuseStrength = saturate(diffuseStrength);
    float diffuse = max(dot(normal, lightDir), 0.0f) * diffuseStrength;

    // 鏡面反射(Blinn-Phong) — roughness で鋭さ、reflectance で強さを制御
    float gloss = 1.0f - roughness;
    float specPower = lerp(8.0f, 128.0f, gloss);
    float specStrength = lerp(0.04f, 1.0f, reflectance);
    float specular = pow(max(dot(normal, halfVec), 0.0f), specPower) * specStrength;

    // 環境光
    float3 ambient = albedo * ambientIntensity;

    // 反射レイを飛ばす（メッシュごとの反射率を使用）
    float3 reflectColor = float3(0, 0, 0);
    if (reflectance > 0.001f && payload.depth < MAX_BOUNCES)
    {
        float3 reflectDir = reflect(WorldRayDirection(), normal);

        RayDesc reflectRay;
        reflectRay.Origin = hitPosition + normal * 0.001f;
        reflectRay.Direction = reflectDir;
        reflectRay.TMin = 0.001f;
        reflectRay.TMax = 10000.0f;

        RayPayload reflectPayload;
        reflectPayload.color = float4(0, 0, 0, 1);
        reflectPayload.depth = payload.depth + 1;

        TraceRay(SceneBVH, RAY_FLAG_NONE, 0xFF, 0, 0, 0, reflectRay, reflectPayload);
        reflectColor = reflectPayload.color.rgb;
    }

    // 最終色の合成（メッシュ単位のパラメータを反映）
    float3 lighting = ambient + (diffuse * albedo + specular) * lightColor.rgb;

    // フレネルで反射寄りにブレンド（斜めほど reflectColor / ハイライトが強くなる）
    float3 finalColor = lighting * (1.0f - fresnel) + (lighting + reflectColor * reflectance) * fresnel;

    payload.color = float4(finalColor, 1.0f);
}

[shader("miss")]
void MyMissShader(inout RayPayload payload)
{
    // 背景色（青空）
    payload.color = float4(0.4f, 0.6f, 0.9f, 1.0f);
}