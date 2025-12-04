#include "directx.h"
#include "effect.h"

#include "shader.h"
#include "texture.h"
#include "camera.h"

//　アスペクト比は16：9

struct EFFECT
{
	bool enable;
	XMFLOAT3 position;
	int frame;
};

EFFECT g_Effect[100];

static ID3D11Buffer* g_VertexBuffer;  // 頂点バッファ

static int g_Texture;

void InitializeEffect()
{
	{  // 頂点バッファ生成
		D3D11_BUFFER_DESC bd{};
		bd.Usage = D3D11_USAGE_DYNAMIC;
		bd.ByteWidth = sizeof(Vertex) * 4;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		DirectXGetDevice()->CreateBuffer(&bd, nullptr, &g_VertexBuffer); //g_VertexBufferはGPUのメモリなのでアクセスできない。
	}


	///////////////////頂点バッファ設定開始///////////////////////
	{
		D3D11_MAPPED_SUBRESOURCE msr;
		DirectXGetDeviceContext()->Map(g_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr); //g_VertexBufferのありかを探す。

		Vertex* v = (Vertex*)msr.pData;

		// 3dにおいては、ピクセル単位ではなくメートル単位になる。
		// xが横方向、yが縦方向、zが奥行方向
		//四つの頂点を作る。頭の中でイメージ

		v[0].position = { -0.5f, 0.5f, 0.0f };
		v[1].position = { 0.5f, 0.5f, 0.0f };
		v[2].position = { -0.5f, -0.5f, 0.0f };
		v[3].position = { 0.5f, -0.5f, 0.0f};

		float tx = 0.0f;
		float ty = 0.0f;
		float tw = 1.0f;
		float th = 1.0f;

		v[0].texcoord = { tx,		ty };
		v[1].texcoord = { tx + tw,	ty };
		v[2].texcoord = { tx,		ty + th };
		v[3].texcoord = { tx + tw,	ty + th };

		v[0].normal = { 0.0f, 0.0f, -1.0f };
		v[1].normal = { 0.0f, 0.0f, -1.0f };
		v[2].normal = { 0.0f, 0.0f, -1.0f };
		v[3].normal = { 0.0f, 0.0f, -1.0f };

		DirectXGetDeviceContext()->Unmap(g_VertexBuffer, 0);
	}
	//////////////頂点バッファ設定終了////////////////////

	g_Texture = TextureLoad(L"asset\\texture\\explosion.png");

	for (int i = 0; i < 100; i++)
	{
		g_Effect[i].enable = false;
	}
}

void FinalizeEffect()
{

}

void UpdateEffect()
{
	for (int i = 0; i < 100; i++)
	{
		if (g_Effect[i].enable == true)
		{
			// コマを加算
			g_Effect[i].frame++;

			if (g_Effect[i].frame >= 16)
			{
				g_Effect[i].enable = false;
			}
		}
	}
}

void DrawEffect()
{
	Shader_Begin();  // シェーダーの設定

	// 頂点バッファ設定
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	DirectXGetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset); //気を付けて　GetじゃなくてSet

	// プリミティブトポロジ設定
	DirectXGetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);  // トライアングルストリップ（連続） つまりZの書き方

	ID3D11ShaderResourceView* texture = GetTexture(g_Texture);
	DirectXGetDeviceContext()->PSSetShaderResources(0, 1, &texture);

	for (int i = 0; i < 100; i++)
	{
		if (!g_Effect[i].enable)
		{
			continue;  // 入れ子構造が減らせるので、こっちのほうが良い
		} 

		int num = g_Effect[i].frame;

		// テクスチャコーディネイトを設定。
		float tx{ (num % 4) / 4.0f };
		float tw{ 1.0f / 4.0f };
		float ty{ (num / 4) / 4.0f };
		float th{ 1.0f / 4.0f };

		///////////////////頂点バッファ設定開始///////////////////////
		{
			D3D11_MAPPED_SUBRESOURCE msr;
			DirectXGetDeviceContext()->Map(g_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr); //g_VertexBufferのありかを探す。

			Vertex* v = (Vertex*)msr.pData;

			// 3dにおいては、ピクセル単位ではなくメートル単位になる。
			// xが横方向、yが縦方向、zが奥行方向
			//四つの頂点を作る。頭の中でイメージ

			v[0].position = { -0.5f, 0.5f, 0.0f };
			v[1].position = { 0.5f, 0.5f, 0.0f };
			v[2].position = { -0.5f, -0.5f, 0.0f };
			v[3].position = { 0.5f, -0.5f, 0.0f };

			v[0].texcoord = { tx,		ty };
			v[1].texcoord = { tx + tw,	ty };
			v[2].texcoord = { tx,		ty + th };
			v[3].texcoord = { tx + tw,	ty + th };

			v[0].normal = { 0.0f, 0.0f, -1.0f };
			v[1].normal = { 0.0f, 0.0f, -1.0f };
			v[2].normal = { 0.0f, 0.0f, -1.0f };
			v[3].normal = { 0.0f, 0.0f, -1.0f };

			DirectXGetDeviceContext()->Unmap(g_VertexBuffer, 0);
		}
		//////////////頂点バッファ設定終了////////////////////

		// ビューマトリクスを取得 カメラのビューマトリクスはカメラが向いている方向そのもの。
		XMMATRIX view = GetCameraViewMatrix();
		// ビューマトリクスの逆行列を求める 掛け算の代わりに割り算をするみたいな感じ。
		XMMATRIX invView = XMMatrixInverse(nullptr, view);

		// 移動成分を消去
		invView.r[3].m128_f32[0] = 0.0f;
		invView.r[3].m128_f32[1] = 0.0f;
		invView.r[3].m128_f32[2] = 0.0f;

		// 頂点シェーダーに変換行列を設定
		XMMATRIX matrix = XMMatrixIdentity();  // 行列を作成　float 4 x 4
		XMMATRIX matrixWorld = XMMatrixIdentity();  // 行列を作成　float 4 x 4

		matrixWorld += XMMatrixScaling(5.0f, 5.0f,5.0f);

		// 回転マトリクス（ビルボード処理）
		matrixWorld *= invView;

		// 移動マトリクス。gpuで計算されている。
		matrixWorld *= XMMatrixTranslation(g_Effect[i].position.x, g_Effect[i].position.y, g_Effect[i].position.z);

		matrix = matrixWorld;

		// ビューマトリクス
		matrix *= GetCameraViewMatrix();

		// プロジェクションマトリクス

		matrix *= GetCameraProjectionMatrix();

		// vertex.hlslのmtxに値を送っている。
		Shader_SetMatrix({ matrix, matrixWorld });

		// ポリゴン描画
		DirectXGetDeviceContext()->Draw(4, 0);

		// SpriteDraw(g_Effect[i].pos.x, g_Effect[i].pos.y, g_Effect[i].size.x, g_Effect[i].size.y, tx, ty, tw, th, g_Texture);
	}
}

void CreateEffect(XMFLOAT3 position)
{
	for (int i = 0; i < 100; i++)
	{
		if (g_Effect[i].enable == false)
		{
			g_Effect[i].enable = true;
			g_Effect[i].position = position;
			g_Effect[i].frame = 0;
			break;
		}
	}
}
