// Shadow Map Generation Vertex Shader
// Transforms vertices to light space for shadow map rendering

// Light view-projection matrix
cbuffer ShadowConstant : register(b0)
{
    matrix lightViewProj;
};

// Object world matrix (from root constants)
cbuffer ObjectConstant : register(b1)
{
    matrix world;
};

struct ShadowVSInput
{
    float4 pos : POSITION;
};

struct ShadowVSOutput
{
    float4 pos : SV_POSITION;
};

ShadowVSOutput main(ShadowVSInput input)
{
    ShadowVSOutput output;
    
    // Transform vertex to world space, then to light space
    float4 worldPos = mul(world, input.pos);
    output.pos = mul(lightViewProj, worldPos);
    
    return output;
}
