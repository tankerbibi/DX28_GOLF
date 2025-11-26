#include "directX.h"
#include "rocket.h"
#include "Keyboard.h"
#include "model.h"
#include "camera.h"
#include "shader.h"
#include "field.h"
#include "goal.h"
#include "main.h"

static MODEL* g_Model = nullptr;

static XMFLOAT3 g_Pos;
static XMFLOAT3 g_Velocity;
static XMFLOAT3 g_Rotation;

static XMFLOAT3 g_Pitch;
static XMFLOAT3 g_Yaw;

static constexpr float g_RocketRadius = 0.2f;

void RocketHitCheck();


void InitializeRocket()
{
	g_Model = ModelLoad("asset\\model\\Rocket.fbx");
	g_Pos = { 0.0f, 10.0f, 0.0f };
	// g_RocketPos = XMFLOAT3(0.0f, 0.0f, 0.0f);  何が違う？
	g_Rotation = { 0.0f, 0.0f, 0.0f };
	g_Velocity = { 0.0f, 0.0f ,0.0f };
	g_Pitch = {0.0f, 0.0f, 0.0f};
	g_Yaw = {0.0f, 0.0f, 0.0f};
}

void FinalizeRocket()
{
	ModelRelease(g_Model);
}

void UpdateRocket()
{
	float deltaTime = 1.0f / 60.0f;

	XMFLOAT3 cameraForward = GetCameraForward();

	cameraForward.y = 0.0f;  // y成分を消す

	// sqrtfはルート 三平方の定理
	float length = sqrtf(cameraForward.x * cameraForward.x
		+ cameraForward.y * cameraForward.y
		+ cameraForward.z * cameraForward.z);

	// Normalize 正規化(長さを１にする) 長さ３のベクトルを長さ３で割るので１になる
	cameraForward.x /= length;
	cameraForward.y /= length;
	cameraForward.z /= length;

	// 力
	XMFLOAT3 force = { 0.0f, 0.0f, 0.0f };

	const XMVECTOR forwardBase = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);  // 方向ベクトル取得
	// --- 1. W/S (前後) 用の前方ベクトルを計算 (ピッチを考慮する) ---
	XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(g_Pitch, g_Yaw, 0.0f);
	XMVECTOR forwardVec_WS = XMVector3TransformNormal(forwardBase, rotationMatrix);

	// --- 2. A/D (左右) 用の"水平"前方ベクトルを計算 (ピッチを 0.0f にする) ---
	XMMATRIX horizontalRotationMatrix = XMMatrixRotationRollPitchYaw(0.0f, g_Yaw, 0.0f);
	XMVECTOR forwardVec_AD = XMVector3TransformNormal(forwardBase, horizontalRotationMatrix);

	// 回転させる
	if (Keyboard_IsKeyDown(KK_A))
	{
		XMMATRIX leftRotationMatrix = XMMatrixRotationRollPitchYaw(0.0f, -XM_PIDIV2, 0);  // 回転マトリクスを取得
		XMVECTOR left = XMVector3TransformNormal(forwardVec_AD, leftRotationMatrix);  // 前方ベクトルを回転
		g_Veloci = XMVectorAdd(velocity, left);
	}
	else if (Keyboard_IsKeyDown(KK_D))
	{
	}
	if (Keyboard_IsKeyDown(KK_W))
	{
	}
	else if (Keyboard_IsKeyDown(KK_S))
	{
	}

	// 力ベクトルの長さ
	float forceLength = sqrtf(force.x * force.x
		+ force.y * force.y
		+ force.z * force.z);

	// 正規化（長さを１にする）
	if (forceLength > 1.0f)
	{
		force.x /= forceLength;
		force.y /= forceLength;
		force.z /= forceLength;
	}

	g_Velocity.x += force.x * 10.0f * deltaTime;
	g_Velocity.z += force.z * 10.0f * deltaTime;


	// ショット
	if (Keyboard_IsKeyTrigger(KK_SPACE))
	{
		g_Velocity.y += 5.0f;  // 撃力
	}

	// 重力
	g_Velocity.y -= 9.8f * deltaTime;

	// 抵抗
	g_Velocity.x -= g_Velocity.x * 2.0f * deltaTime;
	g_Velocity.y -= g_Velocity.y * 0.5f * deltaTime;
	g_Velocity.z -= g_Velocity.z * 2.0f * deltaTime;

	// 移動
	g_Pos.x += g_Velocity.x * deltaTime;
	g_Pos.y += g_Velocity.y * deltaTime;
	g_Pos.z += g_Velocity.z * deltaTime;

	// 衝突判定
	RocketHitCheck();
}

void DrawRocket()
{
	Shader_Begin();  // シェーダーの設定
	// 頂点シェーダーに変換行列を設定

	MATRIX matrix;

	matrix.matrix = XMMatrixIdentity();  // 行列を作成　float 4 x 4
	matrix.matrixWorld = XMMatrixIdentity();  // 行列を作成　float 4 x 4

	matrix.matrixWorld *= XMMatrixScaling(1.0f, 1.0f, 1.0f);  // 拡大縮小マトリクス
	matrix.matrixWorld *= XMMatrixRotationRollPitchYaw(g_Rotation.x, g_Rotation.y, g_Rotation.z);  // 回転マトリクス
	matrix.matrixWorld *= XMMatrixTranslation(g_Pos.x, g_Pos.y, g_Pos.z);  // 移動マトリクス。gpuで計算されている。

	matrix.matrix = matrix.matrixWorld;

	matrix.matrix *= GetCameraViewMatrix();  // ビューマトリクス
	matrix.matrix *= GetCameraProjectionMatrix();  // プロジェクションマトリクス

	Shader_SetMatrix(matrix);
	ModelDraw(g_Model);
}

XMFLOAT3 GetRocketPos()
{
	return g_Pos;
}

void RocketHitCheck()
{
	BLOCK* block = GetFieldBlock();
	float blockRadius = 0.5f;


	float e = 0.5f;  // 跳ね返り係数

	for (int i = 0; i < blockMax; i++)
	{
		// 横方向の当たり判定処理
		if (block[i].pos.y - blockRadius < g_Pos.y &&
			g_Pos.y < block[i].pos.y + blockRadius)  // 横からみた図の状況を作り出している！！
		{
			// x方向
			if (block[i].pos.z - blockRadius < g_Pos.z &&
				g_Pos.z < block[i].pos.z + blockRadius)  // 3次元だから2次元に絞ろう！
			{
				if (block[i].pos.x - blockRadius < g_Pos.x + g_RocketRadius &&
					g_Pos.x - g_RocketRadius < block[i].pos.x + blockRadius)
				{
					if (block[i].pos.x < g_Pos.x)
					{
						// 右
						g_Pos.x = block[i].pos.x + blockRadius + g_RocketRadius;
					}
					else
					{
						// 左
						g_Pos.x = block[i].pos.x - blockRadius - g_RocketRadius;
					}
					g_Velocity.x *= -e;
				}
			}
			// z方向
			else if (block[i].pos.x - blockRadius < g_Pos.x + g_RocketRadius &&
				g_Pos.x < block[i].pos.x + blockRadius)
			{
				if (block[i].pos.z - blockRadius < g_Pos.z + g_RocketRadius &&
					g_Pos.z - g_RocketRadius < block[i].pos.z + blockRadius)
				{
					if (block[i].pos.z < g_Pos.z)
					{
						// 奥
						g_Pos.z = block[i].pos.z + blockRadius + g_RocketRadius;
					}
					else
					{
						// 手前
						g_Pos.z = block[i].pos.z - blockRadius - g_RocketRadius;
					}
				}
			}
		}
		else
			// 縦方向の当たり判定処理
		{
			// 手前　奥
			if (block[i].pos.z - blockRadius < g_Pos.z &&
				g_Pos.z < block[i].pos.z + blockRadius)
			{
				if (block[i].pos.x - blockRadius < g_Pos.x &&
					g_Pos.x < block[i].pos.x + blockRadius)
				{
					if (block[i].pos.y - blockRadius < g_Pos.y + g_RocketRadius &&
						g_Pos.y - g_RocketRadius < block[i].pos.y + blockRadius)
					{
						if (g_Pos.y > block[i].pos.y)
						{
							// 上
							g_Pos.y = block[i].pos.y + blockRadius + g_RocketRadius;

						}
						else
						{
							// 下
							g_Pos.y = block[i].pos.y - blockRadius - g_RocketRadius;
						}
						g_Velocity.y *= -e;
					}
				}
			}
		}
	}
}