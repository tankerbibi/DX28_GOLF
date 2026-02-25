#include "directX.h"
#include "player.h"
#include "mouse.h"
#include "camera.h"
#include "bomb.h"
#include "texture.h"
#include "goal.h"
#include "shader.h"
#include "ball.h"
#include "shadow.h"

static constexpr float bombPositionGapY = 0.5f;
static constexpr float animationDeadZone = 0.1f;

static int g_WalkRight = -1;
static int g_WalkLeft = -1;
static int g_Idle = -1;

Player g_Player;

static ID3D11Buffer* g_VertexBuffer;



void Player::InitializePlayer()
{
	position = {0.0f , posYGap, 0.0f};
	velocity = { 0.0f, 0.0f ,0.0f };
}

void Player::FinalizePlayer()
{
}

void Player::UpdatePlayer()
{	
	SetShadowPosition(position);
	static bool flipFlop = false;
	if (velocity.x < animationDeadZone && velocity.x > -animationDeadZone && velocity.z < animationDeadZone && velocity.z > -animationDeadZone)
	{
		animationCount = 0;
		currentDrawTexture = g_Idle;
	}
	else
	{
		animationCount++;
		if (animationCount > animationFrame)
		{
			flipFlop = !flipFlop;
			animationCount = 0;
			if (flipFlop) currentDrawTexture = g_WalkLeft;
			else currentDrawTexture = g_WalkRight;
		}
	}

	XMFLOAT3 goalPosition = GetGoalPosition();

	XMFLOAT3 forward = {
		goalPosition.x - position.x,
		goalPosition.y - position.y,
		goalPosition.z - position.z
	};

	float length = sqrtf(forward.x * forward.x + forward.y * forward.y + forward.z * forward.z);

	forward = {
		forward.x / length,
		forward.y / length,
		forward.z / length
	};

	velocity.x += forward.x * velocityPower * deltaTime;
	velocity.y = 0.0f;
	velocity.z += forward.z * velocityPower * deltaTime;

	velocity.x -= velocity.x * 1.0f * deltaTime;
	velocity.y -= velocity.y * 1.0f * deltaTime;
	velocity.z -= velocity.z * 1.0f * deltaTime;

	position.x += velocity.x * deltaTime;
	position.y += velocity.y * deltaTime;
	position.z += velocity.z * deltaTime;
}

void Player::DrawPlayer()
{
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	DirectXGetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset); //気を付けて　GetじゃなくてSet

	DirectXGetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);  // トライアングルストリップ（連続） つまりZの書き方

	ID3D11ShaderResourceView* texture = GetTexture(currentDrawTexture);
	DirectXGetDeviceContext()->PSSetShaderResources(0, 1, &texture);

	XMMATRIX view = GetCameraViewMatrix();
	XMMATRIX invView = XMMatrixInverse(nullptr, view);

	invView.r[3].m128_f32[0] = 0.0f;
	invView.r[3].m128_f32[1] = 0.0f;
	invView.r[3].m128_f32[2] = 0.0f;

	invView.r[0].m128_f32[0] = 1.0f;
	invView.r[0].m128_f32[1] = 0.0f;
	invView.r[0].m128_f32[2] = 0.0f;

	invView.r[1].m128_f32[0] = 0.0f;
	invView.r[1].m128_f32[1] = 1.0f;
	invView.r[1].m128_f32[2] = 0.0f;

	XMMATRIX matrix = XMMatrixIdentity();
	XMMATRIX matrixWorld = XMMatrixIdentity();

	matrixWorld *= XMMatrixScaling(10, 10, 10);

	matrixWorld *= invView;

	matrixWorld *= XMMatrixTranslation(position.x, position.y, position.z);

	matrix = matrixWorld;

	matrix *= GetCameraViewMatrix();
	matrix *= GetCameraProjectionMatrix();

	Shader_SetMatrix({ matrix, matrixWorld });

	DirectXGetDeviceContext()->Draw(4, 0);
}

void InitializePlayer()
{
	g_WalkRight = TextureLoad(L"asset\\texture\\WalkRight.png");
	g_WalkLeft = TextureLoad(L"asset\\texture\\WalkLeft.png");
	g_Idle = TextureLoad(L"asset\\texture\\Idle.png");
	g_Player.InitializePlayer();

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

}

void FinalizePlayer()
{
	SAFE_RELEASE(g_VertexBuffer);

	g_Player.FinalizePlayer();
}

void UpdatePlayer()
{
	g_Player.UpdatePlayer();
	// 敵の爆弾に当たった時、ダメージを受ける。

	if (IsMouseTriggered(MOUSE_BUTTON::LEFT))
	{
		// 4. 発射！（カメラの位置から、カメラの正面方向へ）
		XMFLOAT3 cameraForward = GetCameraForward();
		XMFLOAT3 cameraPosition = GetCameraPosition();
		CreateBomb({ cameraPosition.x, cameraPosition.y - bombPositionGapY , cameraPosition.z }, { cameraForward.x * 2000.0f, cameraForward.y * 2000.0f, cameraForward.z * 2000.0f });
		// CreateRocket(GetCameraPosition(), cameraForward);

	}
}

void DrawPlayer()
{
	g_Player.DrawPlayer();
}

XMFLOAT3 GetPlayerPosition()
{
	return g_Player.GetPosition();
}
