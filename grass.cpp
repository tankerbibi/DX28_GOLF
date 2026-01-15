#include "directx.h"
#include "grass.h"
#include "shader.h"
#include "texture.h"
#include "camera.h"
#include <cmath> // atan2を使うために必要

// 頂点バッファ
static ID3D11Buffer* g_VertexBuffer = nullptr;
// テクスチャID
static int g_Texture = -1;

void InitializeGrass()
{
	// 2. 頂点バッファの作成
	D3D11_BUFFER_DESC bd{};
	bd.Usage = D3D11_USAGE_DYNAMIC;      // 形は変わらないのでDEFAULT
	bd.ByteWidth = sizeof(Vertex) * 4;   // 4頂点分
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;               // CPUからの書き込みはしない
	DirectXGetDevice()->CreateBuffer(&bd, nullptr, &g_VertexBuffer);


	// テクスチャ読み込み（草の画像を読み込んでください）
	g_Texture = TextureLoad(L"asset\\texture\\piece_whiteRape.png");
}

void FinalizeGrass()
{
	SAFE_RELEASE(g_VertexBuffer);
}

void UpdateGrass()
{
	// 特に何もしない
}

// 座標を指定して描画する関数に変更
void DrawGrass()
{
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
	{
		//頂点バッファにデータを設定
		D3D11_MAPPED_SUBRESOURCE msr;
		DirectXGetDeviceContext()->Map(g_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr); //g_VertexBufferのありかを探す。

		Vertex* v = (Vertex*)msr.pData;

		// 1. カメラへの方向（角度）を計算
		XMFLOAT3 camPos = GetCameraPosition();
		float dx = camPos.x - x;
		float dz = camPos.z - z;
		float angle = atan2f(dx, dz); // Y軸の回転角度

		// 2. 回転のための sin, cos を準備
		float c = cosf(angle);
		float s = sinf(angle);

		// 3. 草の基本サイズ（幅の半分）
		float halfW = 0.5f; // 幅の半分
		float height = 1.0f; // 高さ

		/* 回転の計算式 (Y軸回転)
		   NewX = LocalX * cos - LocalZ * sin
		   NewZ = LocalX * sin + LocalZ * cos

		   今回は板ポリゴンで LocalZ は常に 0 なので、
		   NewX = LocalX * cos
		   NewZ = LocalX * sin
		   これに、描画したい座標(x, z)を足します。
		*/

		// --- 左上 (LocalX: -0.5) ---
		v[0].position.x = x + (-halfW * c);
		v[0].position.y = y + height;
		v[0].position.z = z + (-halfW * s);

		// --- 右上 (LocalX: +0.5) ---
		v[1].position.x = x + (halfW * c);
		v[1].position.y = y + height;
		v[1].position.z = z + (halfW * s);

		// --- 左下 (LocalX: -0.5) ---
		v[2].position.x = x + (-halfW * c);
		v[2].position.y = y;
		v[2].position.z = z + (-halfW * s);

		// --- 右下 (LocalX: +0.5) ---
		v[3].position.x = x + (halfW * c);
		v[3].position.y = y;
		v[3].position.z = z + (halfW * s);

		v[0].normal = { 0.0f, 0.0f, -1.0f };
		v[1].normal = { 0.0f, 0.0f, -1.0f };
		v[2].normal = { 0.0f, 0.0f, -1.0f };
		v[3].normal = { 0.0f, 0.0f, -1.0f };

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

	auto pContext = DirectXGetDeviceContext();

	// テクスチャ設定
	ID3D11ShaderResourceView* texture = GetTexture(g_Texture);
	pContext->PSSetShaderResources(0, 1, &texture);

	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	pContext->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);

	pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	XMMATRIX matrixWorld = XMMatrixIdentity();
	
	matrixWorld *= XMMatrixScaling(1.0f, 1.0f, 1.0f);

	// 移動（引数で受け取った場所へ）
	matrixWorld *= XMMatrixTranslation(0.0f, 0.0f, 0.0f);

	// 4. シェーダーに行列をセット
	XMMATRIX matrix = matrixWorld;
	matrix *= GetCameraViewMatrix();       // ビュー行列
	matrix *= GetCameraProjectionMatrix(); // プロジェクション行列

	Shader_SetMatrix({ matrix, matrixWorld });



	// 5. 描画
	pContext->Draw(4, 0);
}