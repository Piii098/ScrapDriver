#include"TestType.hlsli"

cbuffer SceneConstant : register(b0)
{
    matrix world;
    matrix view;
    matrix proj;
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