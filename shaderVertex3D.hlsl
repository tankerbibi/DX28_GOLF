
//定数バッファ
cbuffer MatrixBuffer : register(b0)  // b0はスロット番号　shader.cppで引数として送った数字と連携している。
{
    float4x4 mtx;
}

cbuffer LightBuffer : register(b1)
{
    bool lightEnable;
    float3 dummy0;
    float3 lightDirection;
    float dummy1;  // ダミー 16バイト区切りにしなければならないルールを遵守するために存在する。
}

// hlslはc言語ベース
// グローバルイルミネーション すべての物体から反射する光を計算して影を作ることをいう。

//頂点シェーダ
float4 main(in float4 position : POSITION0,
            in float2 texcoord : TEXCOORD0,
            in float3 normal : NORMAL0,  // セマンティクスの名前は自分で設定できるが、ある程度のルールがあるので、最初からあるやつを使ったほうが良い。
            out float2 outTexcoord : TEXCOORD0,
            out float4 outColor : COLOR0) : SV_Position  //inは入力用の引数。　POSITION0、TEXCOORD0はセマンティクス。変数の役割を設定できる。  outは出力用の引数。
{
    // ライト方向ベクトル 長さが１を超えないように正規化する。
    // float3 lightDirection = normalize(float3(0.5, -1.0, 1.0));  // c言語ではfが必要だった。しかしシェーダではいらない。シェーダーでは全部float。容量がどれくらいかはGPU次第。
    
    if (lightEnable)
    {
         // ランバート拡散照明(直接光)
        outColor.rgb = -dot(lightDirection, normal);
    
        outColor.rgb += float3(0.3, 0.2, 0.2);  // 環境光（アンビエントライト）　空が夕日の色なので赤を強めにしてみた。
        outColor.rgb = saturate(outColor.rgb);  // saturateは０以下を０に、１以上を１に制限してくれる
    }
    else
    {
        outColor.rgb = 1.0;
    }
    
    outColor.a = 1.0;
    
    outTexcoord = texcoord;
    return mul(position, mtx);
}