#include "sprite.h"

#include "directx.h"
#include "shader.h"
#include "texture.h"

//ポリゴンにテクスチャを張り付けて、テクスチャを表示する。ポリゴンは３Dの機能である。 ポリゴンとは多角形のことを指す。
/*
四角形のポリゴンを出して、テクスチャを張り付けることが目標。
ポリゴンを出すためにはVertex(頂点)が4コ必要。
*/

struct AlphaBuffer
{
	float alpha{ 1.0f };
	float padding[3]{ 0, 0, 0 };
};

static ID3D11Buffer* g_VertexBuffer;
static int g_Texture;
static AlphaBuffer g_Alpha;
static ID3D11Buffer* g_AlphaBuffer;

void SpriteInitialize()
{
	//頂点バッファ生成
	D3D11_BUFFER_DESC bd{};
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(Vertex) * 8;	//４コ
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	DirectXGetDevice()->CreateBuffer(&bd, nullptr, &g_VertexBuffer); //g_VertexBufferはGPUのメモリなのでアクセスできない。

	D3D11_BUFFER_DESC alphaDesc{};
	alphaDesc.Usage = D3D11_USAGE_DEFAULT;
	alphaDesc.ByteWidth = sizeof(AlphaBuffer);
	alphaDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	alphaDesc.CPUAccessFlags = 0;
	DirectXGetDevice()->CreateBuffer(&alphaDesc, nullptr, &g_AlphaBuffer);

	SetAlpha(1.0f);
}

void SpriteFinalize()
{
	SAFE_RELEASE(g_VertexBuffer);
	SAFE_RELEASE(g_AlphaBuffer);
}

void SpriteDraw(float x, float y, float width, float height, float tx, float ty, float tw, float th, int texId)
{
	ID3D11ShaderResourceView* texture = GetTexture(texId);
	DirectXGetDeviceContext()->PSSetShaderResources(0, 1, &texture);

	//頂点バッファにデータを設定
	D3D11_MAPPED_SUBRESOURCE msr;
	DirectXGetDeviceContext()->Map(g_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr); //g_VertexBufferのありかを探す。

	Vertex* v = (Vertex*)msr.pData;

	//四つの頂点を作る。頭の中でイメージ
	v[0].position = { -width / 2,  -height / 2,				0.0f }; //　左上
	v[1].position = { width / 2, -height / 2,				0.0f };
	v[2].position = { -width / 2,  height / 2,		0.0f };
	v[3].position = { width / 2,  height / 2,		0.0f };

	v[0].texcoord = { tx,		ty };
	v[1].texcoord = { tx + tw,	ty };
	v[2].texcoord = { tx,		ty + th };
	v[3].texcoord = { tx + tw,	ty + th };

	DirectXGetDeviceContext()->Unmap(g_VertexBuffer, 0);

	Shader_Begin();

	//頂点シェーダーに変換行列を設定
	XMMATRIX matrix{ XMMatrixIdentity() };  // 行列を作成　float 4 x 4

	//移動マトリクス
	matrix *= XMMatrixTranslation(x, y, 0.0f); // gpuで計算されている。

	//プロジェクションマトリクス 3dの画面に大きさを合わせるためのマトリクス
	matrix *= XMMatrixOrthographicOffCenterLH(0.0f, screenWidth, screenHeight, 0.0f, 0.0f, 1.0f);
	Shader_SetMatrix(matrix);  // 画面の大きさにうまく合わせてくれる。

	//頂点バッファ設定
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	DirectXGetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset); //気を付けて　GetじゃなくてSet

	//プリミティブトポロジ設定
	DirectXGetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	//ポリゴン描画
	DirectXGetDeviceContext()->Draw(4, 0);  //頂点数

	SetAlpha(1.0f);
}

void SpriteDrawRotation(float x, float y, float width, float height, float tx, float ty, float tw, float th, float rotation, float texId)
{
	ID3D11ShaderResourceView* texture = GetTexture(texId);
	DirectXGetDeviceContext()->PSSetShaderResources(0, 1, &texture);

	//頂点バッファにデータを設定
	D3D11_MAPPED_SUBRESOURCE msr;
	DirectXGetDeviceContext()->Map(g_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr); //g_VertexBufferのありかを探す。

	Vertex* v = (Vertex*)msr.pData;

	//四つの頂点を作る。頭の中でイメージ
	v[0].position = { -width / 2,  -height / 2,				0.0f }; //　左上
	v[1].position = { width / 2, -height / 2,				0.0f };
	v[2].position = { -width / 2,  height / 2,		0.0f };
	v[3].position = { width / 2,  height / 2,		0.0f };

	v[0].texcoord = { tx,		ty };
	v[1].texcoord = { tx + tw,	ty };
	v[2].texcoord = { tx,		ty + th };
	v[3].texcoord = { tx + tw,	ty + th };

	DirectXGetDeviceContext()->Unmap(g_VertexBuffer, 0);

	Shader_Begin();

	//頂点シェーダーに変換行列を設定
	XMMATRIX matrix{ XMMatrixIdentity() };  // 行列を作成　float 4 x 4

	//回転マトリクス
	matrix *= XMMatrixRotationZ(rotation);  // z軸を中心に回転する　行列はかけ合わせることで、合成できる

	//移動マトリクス
	matrix *= XMMatrixTranslation(x, y, 0.0f); // gpuで計算されている。

	//プロジェクションマトリクス 3dの画面に大きさを合わせるためのマトリクス
	matrix *= XMMatrixOrthographicOffCenterLH(0.0f, screenWidth, screenHeight, 0.0f, 0.0f, 1.0f);
	Shader_SetMatrix(matrix);  // 画面の大きさにうまく合わせてくれる。

	//頂点バッファ設定
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	DirectXGetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset); //気を付けて　GetじゃなくてSet

	//プリミティブトポロジ設定
	DirectXGetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	//ポリゴン描画
	DirectXGetDeviceContext()->Draw(4, 0);  //頂点数

	SetAlpha(1.0f);
}


void SetAlpha(float alpha)
{
	if (alpha < 0.0f)
	{
		alpha = 0.0f;
	}
	if (alpha > 1.0f)
	{
		alpha = 1.0f;
	}
	g_Alpha.alpha = alpha;
	DirectXGetDeviceContext()->UpdateSubresource(g_AlphaBuffer, 0, nullptr, &g_Alpha, 0, 0);
	DirectXGetDeviceContext()->PSSetConstantBuffers(0, 1, &g_AlphaBuffer);
}