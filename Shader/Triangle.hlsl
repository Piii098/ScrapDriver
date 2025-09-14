// 三角形用のシンプルなシェーダー

struct VertexInput
{
    float3 position : POSITION;
    float4 color : COLOR;
};

struct PixelInput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

// 頂点シェーダー
PixelInput TriangleVertexShader(VertexInput input)
{
    PixelInput output;
    output.position = float4(input.position, 1.0f);
    output.color = input.color;
    return output;
}

// ピクセルシェーダー
float4 TrianglePixelShader(PixelInput input) : SV_TARGET
{
    return input.color;
}
