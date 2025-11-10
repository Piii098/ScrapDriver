struct VSOutput
{
    float4 svpos : SV_POSITION; // 
    float4 color : COLOR; // 
    float2 uv : TEXCOORD; // 
};

SamplerState smp : register(s0);
Texture2D _MainTex : register(t0);

float4 PS(VSOutput input) : SV_TARGET
{
    // テクスチャをサンプリング
    float4 texColor = _MainTex.Sample(smp, input.uv);
    
    // デバッグ: UV座標を色として表示（一時的）
    // return float4(input.uv, 0.0, 1.0);
    
    // デバッグ: テクスチャサンプリング結果を確認
    // RGB値をそのまま表示し、アルファ値を青チャンネルに表示
    return float4(texColor.rgb, 1.0);
    
    // テクスチャが読み込まれているかチェック
    // if (texColor.a < 0.001) return float4(1.0, 0.0, 1.0, 1.0); // マゼンタ = テクスチャなし
    
    // return texColor;
}

