#include "directX.h"
#include "start.h"
#include "Keyboard.h"
#include "model.h"
#include "camera.h"
#include "shader.h"
#include "field.h"
#include "texture.h"

static int g_Texture;

static ID3D11Buffer* g_VertexBuffer;

static XMFLOAT3 g_FlagPosition;
static XMFLOAT3 g_Velocity;
static XMFLOAT3 g_Rotation;

static XMFLOAT3 g_StartPosition;


static float g_Shake;
static float g_ShakeTime;

void InitializeStart()
{
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

	g_Texture = TextureLoad(L"asset\\texture\\Start.png");
	g_FlagPosition = { 0.0f, 0.0f, 0.0f};
	g_StartPosition = { 0.0f, 0.0f, 0.0f};
	g_Rotation = { 0.0f, 0.0f, 0.0f };
}

void FinalizeStart()
{
	SAFE_RELEASE(g_VertexBuffer);

}

void UpdateStart()
{
	g_ShakeTime += 1.0f / 60.0f;
	if (g_ShakeTime > XM_2PI) g_ShakeTime = 0.0f;

	g_Shake -= 0.1f;
	if (g_Shake < 0.0f) g_Shake = 0.0f;

	g_FlagPosition.y += sinf(g_ShakeTime * 90.0f) * 0.1f * g_Shake;
}

void DrawStart()
{
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

		// 頂点シェーダーに変換行列を設定
		XMMATRIX matrix = XMMatrixIdentity();  // 行列を作成　float 4 x 4
		XMMATRIX matrixWorld = XMMatrixIdentity();
		// 裏側を描画するための変換行列
		XMMATRIX matrixFlip = XMMatrixIdentity();  // 行列を作成　float 4 x 4
		XMMATRIX matrixWorldFlip = XMMatrixIdentity();

		matrixWorld *= XMMatrixScaling(37.0f, 57.0f, 17.0f);
		matrixWorldFlip *= XMMatrixScaling(37.0f, 57.0f, 17.0f);
		// 反転させる
		matrixWorldFlip *= XMMatrixRotationY(XM_PI);

		// 移動マトリクス。gpuで計算されている。
		matrixWorld *= XMMatrixTranslation(g_FlagPosition.x, g_FlagPosition.y, g_FlagPosition.z);
		matrixWorldFlip *= XMMatrixTranslation(g_FlagPosition.x, g_FlagPosition.y, g_FlagPosition.z);

		matrix = matrixWorld;
		matrixFlip = matrixWorldFlip;
		// ビューマトリクス
		matrix *= GetCameraViewMatrix();
		matrixFlip *= GetCameraViewMatrix();
		// プロジェクションマトリクス
		matrix *= GetCameraProjectionMatrix();
		matrixFlip *= GetCameraProjectionMatrix();
		// 表面描画 vertex.hlslのmtxに値を送っている。
		Shader_SetMatrix({ matrix, matrixWorld });
		DirectXGetDeviceContext()->Draw(4, 0);
		// 裏面描画
		Shader_SetMatrix({ matrixFlip, matrixWorldFlip });
		DirectXGetDeviceContext()->Draw(4, 0);
	}
}

XMFLOAT3 GetStartPosition()
{
	return g_StartPosition;
}

void SetStartPosition(XMFLOAT3 position)
{
	g_StartPosition = position;
}

void SetStartFlagPosition(XMFLOAT3 position)
{
	g_FlagPosition = position;
}



void SetStartShake(float Shake)
{
	g_Shake = Shake;
	g_ShakeTime = 0.0f;
}
