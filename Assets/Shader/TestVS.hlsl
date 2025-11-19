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
    output.svpos = mul(mul(mul(proj, view), world), pos);
    normal.w = 0;
    output.normal = mul(world, normal);
    return output;
}