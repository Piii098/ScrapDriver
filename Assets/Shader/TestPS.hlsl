#include"TestType.hlsli"

Texture2D<float4> tex : register(t0);
SamplerState smp : register(s0);

cbuffer MaterialConstant : register(b2)
{
    float4 baseColor;
};

float4 PS(TestType input) : SV_TARGET
{
    float3 light = normalize(float3(1, 1, 1));
    float brightness = dot(-light, input.normal.xyz);
   
    return tex.Sample(smp, input.uv) * baseColor;
    //return float4(input.uv.xy, 1, 1);
}