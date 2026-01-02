#include"TestType.hlsli"

// シーン共通（ビュー・プロジェクション）
cbuffer SceneConstant : register(b0)
{
    matrix view;
    matrix proj;
};

// オブジェクト個別（ワールド行列）
// RootSignature 側で b1 を 32bit ルート定数としてバインドする
cbuffer ObjectConstant : register(b1)
{
    matrix world;
};

TestType VS(float4 pos : POSITION, float2 uv : TEXCOORD, float4 normal : NORMAL)
{
    TestType output;
    output.uv = uv;
    
    // Transform to world space
    float4 worldPos = mul(world, pos);
    output.worldPos = worldPos.xyz;
    
    // Transform to clip space
    output.svpos = mul(mul(proj, view), worldPos);
    
    // Transform normal to world space
    normal.w = 0;
    output.normal = mul(world, normal);
    
    return output;
}