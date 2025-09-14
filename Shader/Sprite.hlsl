
cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

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

PixelInput VSMain(VertexInput input)
{
    PixelInput output;
    
    output.position = float4(input.position, 1.0f);
    
    // 座標変換を有効に戻す
    // 乗算の順序: ワールド -> ビュー -> プロジェクション
    //output.position = mul(float4(input.position, 1.0f), worldMatrix);
    //output.position = mul(output.position, viewMatrix);
    //output.position = mul(output.position, projectionMatrix);
    
    output.color = input.color;
    return output;

}

float4 PSMain(PixelInput input) : SV_TARGET
{
    return input.color;
}