
//cbuffer MatrixBuffer{
//    matrix worldMatrix;
//    matrix viewMatrix;
//    matrix projectionMatrix;
//};

//struct VertexType {
//    float4 Pos : POSITION;
//    float2 Color : COLOR;
//};

//struct PixelType{
//    float4 Pos : SV_POSITION;
//    float2 Color : COLOR;
//};

//PixelType ColorVertexShader(VertexType input){
//    PixelInputType output;
   
//    input.position.w = 1.0f;

//    output.position = mul(input.position, worldMatrix);
//    output.position = mul(output.position, viewMatrix);
//    output.position = mul(output.position, projectionMatrix);
    
//    output.color = input.color;
    
//    return output;
//}   

//float4 ColorPixelShader(PixelType input) : SV_TARGET{
//    return input.Color;
//}