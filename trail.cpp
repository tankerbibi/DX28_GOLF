#include "directx.h"

#include "trail.h"

#include "shader.h"
#include "texture.h"
#include "camera.h"

//　アスペクト比は16：9

static ID3D11Buffer* g_VertexBuffer;

// 頂点数
static constexpr int trailLength = 30;

static XMFLOAT3 g_TrailPosition[trailLength];

static int g_Texture;

void InitializeTrail()
{
	{  // 頂点バッファ生成
		D3D11_BUFFER_DESC bd{};
		bd.Usage = D3D11_USAGE_DYNAMIC;
		// 頂点は保存ポジションの２倍分必要。	
		bd.ByteWidth = sizeof(Vertex) * (trailLength * 2 - 2);
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		DirectXGetDevice()->CreateBuffer(&bd, nullptr, &g_VertexBuffer); //g_VertexBufferはGPUのメモリなのでアクセスできない。
	}

	g_Texture = TextureLoad(L"asset\\texture\\shadow.png");
	ResetTrailPosition({0.0f, 0.0f, 0.0f});
}

void FinalizeTrail()
{
	SAFE_RELEASE(g_VertexBuffer);

}

void UpdateTrail()
{

}

void DrawTrail()
{
	///////////////////頂点バッファ設定開始///////////////////////
	{
		D3D11_MAPPED_SUBRESOURCE msr;
		DirectXGetDeviceContext()->Map(g_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr); //g_VertexBufferのありかを探す。

		Vertex* v = (Vertex*)msr.pData;

		XMFLOAT3 cameraPosition = GetCameraPosition();

		for (int i = 0; i < trailLength - 1; i++)
		{
			// TrailPositionからCameraPositionを指すベクトルを取得
			XMFLOAT3 cameraDirection;
			cameraDirection.x = cameraPosition.x - g_TrailPosition[i].x;
			cameraDirection.y = cameraPosition.y - g_TrailPosition[i].y;
			cameraDirection.z = cameraPosition.z - g_TrailPosition[i].z;

			XMFLOAT3 trailDirection;
			trailDirection.x = g_TrailPosition[i + 1].x - g_TrailPosition[i].x;
			trailDirection.y = g_TrailPosition[i + 1].y - g_TrailPosition[i].y;
			trailDirection.z = g_TrailPosition[i + 1].z - g_TrailPosition[i].z;

			// 外積を求める　ベクトルxベクトルのxは、外積を表す。
			XMFLOAT3 crossProduct;
			crossProduct.x = cameraDirection.y * trailDirection.z - cameraDirection.z * trailDirection.y;
			crossProduct.y = cameraDirection.z * trailDirection.x - cameraDirection.x * trailDirection.z;
			crossProduct.z = cameraDirection.x * trailDirection.y - cameraDirection.y * trailDirection.x;

			// 外積の長さを求める
			float crossProductLength = sqrtf(crossProduct.x * crossProduct.x
				+ crossProduct.y * crossProduct.y
				+ crossProduct.z * crossProduct.z);

			// 正規化をする
			crossProduct.x /= crossProductLength;
			crossProduct.y /= crossProductLength;
			crossProduct.z /= crossProductLength;

			float t = (float)(trailLength - 1 - i) / (trailLength - 1); // 0.0 ～ 1.0
			float width = (1.0f - t) * 1.5f;        // 先端ほど0に近づく

			v[i * 2 + 0].position.x = g_TrailPosition[i].x + crossProduct.x * width;
			v[i * 2 + 0].position.y = g_TrailPosition[i].y + crossProduct.y * width;
			v[i * 2 + 0].position.z = g_TrailPosition[i].z + crossProduct.z * width;
																			  
			v[i * 2 + 1].position.x = g_TrailPosition[i].x - crossProduct.x * width;
			v[i * 2 + 1].position.y = g_TrailPosition[i].y - crossProduct.y * width;
			v[i * 2 + 1].position.z = g_TrailPosition[i].z - crossProduct.z * width;

			v[i * 2 + 0].texcoord = { 0.5f, 1.0f };
			v[i * 2 + 1].texcoord = { 0.5f, 0.0f };

			v[i * 2 + 0].normal = { 0.0f, 0.0f, -1.0f };
			v[i * 2 + 1].normal = { 0.0f, 0.0f, -1.0f };
		}

		DirectXGetDeviceContext()->Unmap(g_VertexBuffer, 0);
	}
	//////////////頂点バッファ設定終了////////////////////

	ID3D11ShaderResourceView* texture = GetTexture(g_Texture);
	DirectXGetDeviceContext()->PSSetShaderResources(0, 1, &texture);

	Shader_Begin();  // シェーダーの設定

	// 頂点バッファ設定
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	DirectXGetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset); //気を付けて　GetじゃなくてSet

	// プリミティブトポロジ設定
	DirectXGetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);  // トライアングルストリップ（連続） つまりZの書き方

	// 頂点シェーダーに変換行列を設定
	XMMATRIX matrix = XMMatrixIdentity();  // 行列を作成　float 4 x 4
	XMMATRIX matrixWorld = XMMatrixIdentity();  // 行列を作成　float 4 x 4


	matrix = matrixWorld;

	// ビューマトリクス
	matrix *= GetCameraViewMatrix();

	// プロジェクションマトリクス
	matrix *= GetCameraProjectionMatrix();

	// vertex.hlslのmtxに値を送っている。
	Shader_SetMatrix({ matrix, matrixWorld });

	// ポリゴン描画
	DirectXGetDeviceContext()->Draw(trailLength * 2 - 2, 0);
}

void SetTrailPosition(XMFLOAT3 position)
{
	for (int i = 0; i < trailLength - 1; i++)
	{
		// 自分より前のインデックスの内容を、自分のインデックスに保存する。
		g_TrailPosition[i] = g_TrailPosition[i + 1];
	}

	// 一番最後のインデックスに最新のポジションを入れる。
	g_TrailPosition[trailLength - 1] = position;
}

void ResetTrailPosition(XMFLOAT3 position)
{
	for (int i = 0; i < trailLength - 1; i++)
	{
		g_TrailPosition[i] = position;
	}
}

////////////////////////////////////////////////////////////
// 静的メンバ変数の実体定義
int Trail::s_Texture = -1;

// コンストラクタ
Trail::Trail()
{
	// 座標配列をゼロクリア
	ZeroMemory(m_History, sizeof(XMFLOAT3) * TRAIL_LENGTH);
}

Trail::~Trail()
{
	Finalize();
}

// 全トレイル共通のリソース（テクスチャ）を読み込む
void Trail::LoadCommonResources()
{
	if (s_Texture == -1) {
		s_Texture = TextureLoad(L"asset\\texture\\shadow.png");
	}
}

// 共通リソースの開放
void Trail::UnloadCommonResources()
{
	// Textureシステム側で管理されている場合、明示的なUnloadが必要なければ空でも可
	// ここでは管理IDをリセットするのみとします
	s_Texture = -1;
}

void Trail::Initialize()
{
	//// 頂点バッファ生成
	//D3D11_BUFFER_DESC bd{};
	//bd.Usage = D3D11_USAGE_DYNAMIC;
	//// 頂点は保存ポジションの２倍分必要。
	//bd.ByteWidth = sizeof(Vertex) * (TRAIL_LENGTH * 2 - 2);
	//bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	//bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	//// メンバ変数 m_VertexBuffer に作成
	//DirectXGetDevice()->CreateBuffer(&bd, nullptr, &m_VertexBuffer);
}

void Trail::Finalize()
{
	/*SAFE_RELEASE(m_VertexBuffer);*/
}

void Trail::Update(XMFLOAT3 position)
{
	// 履歴の更新
	for (int i = 0; i < TRAIL_LENGTH - 1; i++)
	{
		// 自分より前のインデックスの内容を、自分のインデックスに保存する。
		m_History[i] = m_History[i + 1];
	}

	// 一番最後のインデックスに最新のポジションを入れる。
	m_History[TRAIL_LENGTH - 1] = position;
}

void Trail::Reset(XMFLOAT3 position)
{
	for (int i = 0; i < TRAIL_LENGTH; i++)
	{
		m_History[i] = position;
	}
}

void Trail::Draw()
{
	if (!g_VertexBuffer) return;

	///////////////////頂点バッファ設定開始///////////////////////
	{
		D3D11_MAPPED_SUBRESOURCE msr;
		// メンバ変数 m_VertexBuffer をマップする
		DirectXGetDeviceContext()->Map(g_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);

		Vertex* v = (Vertex*)msr.pData;

		XMFLOAT3 cameraPosition = GetCameraPosition();

		for (int i = 0; i < TRAIL_LENGTH - 1; i++)
		{
			// m_Historyを使用
			XMFLOAT3 pCurrent = m_History[i];
			XMFLOAT3 pNext = m_History[i + 1];

			// TrailPositionからCameraPositionを指すベクトルを取得
			XMFLOAT3 cameraDirection;
			cameraDirection.x = cameraPosition.x - pCurrent.x;
			cameraDirection.y = cameraPosition.y - pCurrent.y;
			cameraDirection.z = cameraPosition.z - pCurrent.z;

			XMFLOAT3 trailDirection;
			trailDirection.x = pNext.x - pCurrent.x;
			trailDirection.y = pNext.y - pCurrent.y;
			trailDirection.z = pNext.z - pCurrent.z;

			// 外積を求める
			XMFLOAT3 crossProduct;
			crossProduct.x = cameraDirection.y * trailDirection.z - cameraDirection.z * trailDirection.y;
			crossProduct.y = cameraDirection.z * trailDirection.x - cameraDirection.x * trailDirection.z;
			crossProduct.z = cameraDirection.x * trailDirection.y - cameraDirection.y * trailDirection.x;

			// 外積の長さ
			float crossProductLength = sqrtf(crossProduct.x * crossProduct.x
				+ crossProduct.y * crossProduct.y
				+ crossProduct.z * crossProduct.z);

			// 正規化（ゼロ除算対策を入れると安全ですが、元のコードに準拠します）
			if (crossProductLength > 0.0001f) {
				crossProduct.x /= crossProductLength;
				crossProduct.y /= crossProductLength;
				crossProduct.z /= crossProductLength;
			}

			float t = (float)(TRAIL_LENGTH - 1 - i) / (TRAIL_LENGTH - 1);
			float width = (1.0f - t) * 1.5f;

			v[i * 2 + 0].position.x = pCurrent.x + crossProduct.x * width;
			v[i * 2 + 0].position.y = pCurrent.y + crossProduct.y * width;
			v[i * 2 + 0].position.z = pCurrent.z + crossProduct.z * width;

			v[i * 2 + 1].position.x = pCurrent.x - crossProduct.x * width;
			v[i * 2 + 1].position.y = pCurrent.y - crossProduct.y * width;
			v[i * 2 + 1].position.z = pCurrent.z - crossProduct.z * width;

			v[i * 2 + 0].texcoord = { 0.5f, 1.0f };
			v[i * 2 + 1].texcoord = { 0.5f, 0.0f };

			v[i * 2 + 0].normal = { 0.0f, 0.0f, -1.0f };
			v[i * 2 + 1].normal = { 0.0f, 0.0f, -1.0f };
		}

		DirectXGetDeviceContext()->Unmap(g_VertexBuffer, 0);
	}
	//////////////頂点バッファ設定終了////////////////////

	// 共通テクスチャを使用
	ID3D11ShaderResourceView* texture = GetTexture(s_Texture);
	DirectXGetDeviceContext()->PSSetShaderResources(0, 1, &texture);

	Shader_Begin();

	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	// メンバ変数の頂点バッファをセット
	DirectXGetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);

	DirectXGetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	XMMATRIX matrix = XMMatrixIdentity();
	XMMATRIX matrixWorld = XMMatrixIdentity();

	matrix = matrixWorld;
	matrix *= GetCameraViewMatrix();
	matrix *= GetCameraProjectionMatrix();

	Shader_SetMatrix({ matrix, matrixWorld });

	DirectXGetDeviceContext()->Draw(TRAIL_LENGTH * 2 - 2, 0);
}
