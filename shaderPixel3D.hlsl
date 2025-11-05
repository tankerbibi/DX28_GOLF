Texture2D tex : register(t0); // 画像が入っている
SamplerState samplerState : register(s0);
//ピクセル　１ドットの色を計算するもの。

cbuffer g_AlphaBuffer : register(b0)
{
    float alpha;
}

float4 main(in float4 position : SV_Position, // 一万個ピクセルがあるなら、一万回ピクセルが呼ばれる。
            in float2 texcoord : TEXCOORD0,
            in float4 color : COLOR0) : SV_TARGET
{
    float4 c = tex.Sample(samplerState, texcoord) * color;
    //color.r *= 1.0f;
    //color.g *= 1.0f;
    //color.b *= 1.0f;
    // color.a *= alpha;
    
    return c; //色をsamplerState設定で着色して！
}