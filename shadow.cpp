#include "directx.h"

#include "shadow.h"

#include "shader.h"
#include "texture.h"
#include "camera.h"
#include "field.h"

static int g_Texture;
static ID3D11Buffer* g_VertexBuffer;  // 頂点バッファ
static ID3D11Buffer* g_InstanceBuffer;
static XMFLOAT3 g_Position;

void ShadowHitCheck();

void InitializeShadow()
{
	// インスタンスバッファ生成
	{
		D3D11_BUFFER_DESC desc = {};
		// 1個分のサイズ
		desc.ByteWidth = sizeof(InstanceData);
		// 毎フレーム更新するため動的に設定
		desc.Usage = D3D11_USAGE_DYNAMIC;
		// 頂点バッファとして扱う
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		// CPUから書き込み可能にする
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		DirectXGetDevice()->CreateBuffer(&desc, nullptr, &g_InstanceBuffer);
	}

	// 頂点バッファ生成
	{
		D3D11_BUFFER_DESC bd{};
		bd.Usage = D3D11_USAGE_DYNAMIC;
		// 1個分のサイズ
		bd.ByteWidth = sizeof(Vertex);
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		DirectXGetDevice()->CreateBuffer(&bd, nullptr, &g_VertexBuffer);
	}


	// 頂点バッファにデータを設定
	{
		D3D11_MAPPED_SUBRESOURCE msr;
		DirectXGetDeviceContext()->Map(g_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr); //g_VertexBufferのありかを探す。

		Vertex* v = (Vertex*)msr.pData;

		v[0].position = { -0.5f, 0.0f, 0.5f};
		v[1].position = { 0.5f, 0.0f, 0.5f};
		v[2].position = { -0.5f, 0.0f, -0.5f};
		v[3].position = { 0.5f, 0.0f, -0.5f};

		v[0].normal = { 0.0f, 1.0f, 0.0f };
		v[1].normal = { 0.0f, 1.0f, 0.0f };
		v[2].normal = { 0.0f, 1.0f, 0.0f };
		v[3].normal = { 0.0f, 1.0f, 0.0f };
		
		float tx = 0.0f;
		float ty = 0.0f;
		float tw = 1.0f;
		float th = 1.0f;

		v[0].texcoord = { tx,		ty };
		v[1].texcoord = { tx + tw,	ty };
		v[2].texcoord = { tx,		ty + th };
		v[3].texcoord = { tx + tw,	ty + th };

		DirectXGetDeviceContext()->Unmap(g_VertexBuffer, 0);
	}
	g_Texture = TextureLoad(L"asset\\texture\\shadow.png");

	g_Position = {-1.0f, 0.0f, 0.0f};
}

void FinalizeShadow()
{
	SAFE_RELEASE(g_VertexBuffer);
	SAFE_RELEASE(g_InstanceBuffer);
}

void UpdateShadow()
{

}

void DrawShadow()
{
	auto pContext = DirectXGetDeviceContext();

	// テクスチャ設定
	ID3D11ShaderResourceView* texture = GetTexture(g_Texture);
	DirectXGetDeviceContext()->PSSetShaderResources(0, 1, &texture);
	
	// バッファをロック
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	DirectXGetDeviceContext()->Map(g_InstanceBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

	auto* data = (InstanceData*)mappedResource.pData;

	XMMATRIX world = XMMatrixIdentity();
	world *= XMMatrixTranslation(g_Position.x, g_Position.y, g_Position.z);
	world *= XMMatrixScaling(5.0f, 5.0f, 5.0f);
	// データを入れる
	data[0].worldMatrix = world;

	DirectXGetDeviceContext()->Unmap(g_InstanceBuffer, 0);

	// 頂点バッファ設定
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	DirectXGetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset); //気を付けて　GetじゃなくてSet

	// インスタンスバッファ設定
	UINT instanceStride = sizeof(InstanceData);
	UINT instanceOffset = 0;
	DirectXGetDeviceContext()->IASetVertexBuffers(1, 1, &g_InstanceBuffer, &instanceStride, &instanceOffset);

	// プリミティブトポロジ設定
	DirectXGetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);  // トライアングルストリップ（連続） つまりZの書き方

	// Identityは一番最初にしなければならない。
	XMMATRIX matrix = XMMatrixIdentity();  // 行列を作成　float 4 x 4
	XMMATRIX matrixWorld = XMMatrixIdentity();  // 行列を作成　float 4 x 4

	// ビューマトリクス
	matrix *= GetCameraViewMatrix();

	// プロジェクションマトリクス
	matrix *= GetCameraProjectionMatrix();

	// 頂点シェーダーに変換行列を設定
	Shader_SetMatrix({ matrix, matrixWorld });

	// ポリゴン描画
	// pContext()->Draw(4, 0);

	DirectXGetDeviceContext()->DrawInstanced(4, 1, 0, 0);
}

void SetShadowPosition(XMFLOAT3 position)
{
	g_Position = position;
	ShadowHitCheck();
}

void ShadowHitCheck()
{
	BLOCK* block = GetFieldBlock();
	float blockRadius = 0.5f;
	float shadowY = -100.0f;

	for (int i = 0; i < blockMax; i++)
	{
		if (block[i].pos.z - blockRadius < g_Position.z &&
			g_Position.z < block[i].pos.z + blockRadius)
		{
			if (block[i].pos.x - blockRadius < g_Position.z &&
				g_Position.x < block[i].pos.x + blockRadius)
			{
				if (block[i].pos.y + blockRadius < g_Position.y)
				{
					float y = (block[i].pos.y + blockRadius) + 0.001f;

					// 一番高いy座標を保存する。
					if (y > shadowY) shadowY = y;
				}
			}
		}
	}
	g_Position.y = shadowY;
}
