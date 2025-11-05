
//定数バッファ
float4x4 mtx;

//頂点シェーダ
float4 main(in float4 position : POSITION0,
            in float2 texcoord : TEXCOORD0,
            in float3 normal : NORMAL0,  // セマンティクスの名前は自分で設定できるが、ある程度のルールがあるので、最初からあるやつを使ったほうが良い。
            out float2 outTexcoord : TEXCOORD0,
            out float4 outColor : COLOR0) : SV_Position //inは入力用の引数。　POSITION0、TEXCOORD0はセマンティクス。変数の役割を設定できる。  outは出力用の引数。
{
    // ライト方向ベクトル 長さが１を超えないように正規化する。
    float3 lightDirection = normalize(float3(0.5, -1.0, 1.0)); // c言語ではfが必要だった。しかしシェーダではいらない。シェーダーでは全部float。容量がどれくらいかはGPU次第。
    
    // ランバート拡散照明
    outColor.rgb = saturate(-dot(lightDirection, normal));
    outColor.a = 1.0;
    outTexcoord = texcoord;
    return mul(position, mtx);
}