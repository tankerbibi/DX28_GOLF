#include "directX.h"
#include "goal.h"
#include "Keyboard.h"
#include "model.h"
#include "camera.h"
#include "shader.h"
#include "field.h"
#include "texture.h"

static int g_Texture = -1;

static ID3D11Buffer* g_VertexBuffer;

static XMFLOAT3 g_FlagPosition;
static XMFLOAT3 g_Velocity;
static XMFLOAT3 g_Rotation;

static XMFLOAT3 g_GoalPosition;

static float g_Shake;
static float g_ShakeTime;

void InitializeGoal()
{
	g_Texture = TextureLoad(L"asset\\texture\\goal.png");
	g_FlagPosition = { 0.0f, 0.0f, 0.0f};
	g_Velocity = { 0.0f, 0.0f, 0.0f };
	g_Rotation = { 0.0f, 0.0f, 0.0f };

	{
		// 頂点バッファの作成
		D3D11_BUFFER_DESC bd{};
		bd.Usage = D3D11_USAGE_DYNAMIC;
		// 4頂点分
		bd.ByteWidth = sizeof(Vertex) * 4;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		DirectXGetDevice()->CreateBuffer(&bd, nullptr, &g_VertexBuffer);
	}

	///////////////////頂点バッファ設定開始///////////////////////
	{
		D3D11_MAPPED_SUBRESOURCE msr;
		DirectXGetDeviceContext()->Map(g_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr); //g_VertexBufferのありかを探す。

		Vertex* v = (Vertex*)msr.pData;

		// 3dにおいては、ピクセル単位ではなくメートル単位になる。
		// xが横方向、yが縦方向、zが奥行方向
		//四つの頂点を作る。頭の中でイメージ

		float tx = 0.0;
		float ty = 0.0f;
		float tw = 1.0f;
		float th = 1.0f;

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
}

void FinalizeGoal()
{
	SAFE_RELEASE(g_VertexBuffer);
}

void UpdateGoal()
{
	g_ShakeTime += 1.0f / 60.0f;
	if (g_ShakeTime > XM_2PI) g_ShakeTime = 0.0f;

	g_Shake -= 0.1f;
	if (g_Shake < 0.0f) g_Shake = 0.0f;

	g_FlagPosition.y += sinf(g_ShakeTime * 90.0f) * 0.1f * g_Shake;
}

void DrawGoal()
{
	// 頂点バッファ設定
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	DirectXGetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset); //気を付けて　GetじゃなくてSet

	// プリミティブトポロジ設定
	DirectXGetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);  // トライアングルストリップ（連続） つまりZの書き方

	ID3D11ShaderResourceView* texture = GetTexture(g_Texture);
	DirectXGetDeviceContext()->PSSetShaderResources(0, 1, &texture);

	// ビューマトリクスを取得 カメラのビューマトリクスはカメラが向いている方向そのもの。
	XMMATRIX view = GetCameraViewMatrix();
	// ビューマトリクスの逆行列を求める 掛け算の代わりに割り算をするみたいな感じ。
	XMMATRIX invView = XMMatrixInverse(nullptr, view);

	// 移動成分を消去
	invView.r[3].m128_f32[0] = 0.0f;
	invView.r[3].m128_f32[1] = 0.0f;
	invView.r[3].m128_f32[2] = 0.0f;

	invView.r[0].m128_f32[1] = 0; // 上下方向の成分を0にする
	invView.r[1] = XMVectorSet(0, 1, 0, 0); // 上向きは常に垂直(0,1,0)固定
	invView.r[2].m128_f32[1] = 0;

	// 頂点シェーダーに変換行列を設定
	XMMATRIX matrix = XMMatrixIdentity();  // 行列を作成　float 4 x 4
	XMMATRIX matrixWorld = XMMatrixIdentity();  // 行列を作成　float 4 x 4

	matrixWorld *= XMMatrixScaling(37.0f, 57.0f, 17.0f);

	// 回転マトリクス（ビルボード処理）
	matrixWorld *= invView;

	// 移動マトリクス。gpuで計算されている。
	matrixWorld *= XMMatrixTranslation(g_FlagPosition.x, g_FlagPosition.y, g_FlagPosition.z);

	matrix = matrixWorld;

	// ビューマトリクス
	matrix *= GetCameraViewMatrix();

	// プロジェクションマトリクス
	matrix *= GetCameraProjectionMatrix();

	// vertex.hlslのmtxに値を送っている。
	Shader_SetMatrix({ matrix, matrixWorld });

	// ポリゴン描画
	DirectXGetDeviceContext()->Draw(4, 0);
}

XMFLOAT3 GetGoalPosition()
{
	return g_GoalPosition;
}

void SetGoalPosition(XMFLOAT3 position)
{
	g_GoalPosition = position;
}

void SetGoalFlagPosition(XMFLOAT3 position)
{
	g_FlagPosition = position;
}


void SetGoalShake(float Shake)
{
	g_Shake = Shake;
	g_ShakeTime = 0.0f;
}
