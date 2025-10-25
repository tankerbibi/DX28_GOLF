
//定数バッファ
float4x4 mtx;

//頂点シェーダ
float4 main(in float4 position : POSITION0,
            in float2 texcoord : TEXCOORD0,
            out float2 outTexcoord : TEXCORD0) : SV_Position //inは入力用の引数。　POSITION0、SV_Positionはセマンティクス。変数の役割を設定できる。
{
    outTexcoord = texcoord;
    return mul(position, mtx);
}