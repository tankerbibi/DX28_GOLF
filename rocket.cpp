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
static XMFLOAT3 g_TargetPos;

static float g_Pitch;
static float g_Yaw;

static constexpr float g_RocketRadius = 0.2f;

void RocketHitCheck();


void InitializeRocket()
{
	g_Model = ModelLoad("asset\\model\\Rocket.fbx");
	g_Pos = { 0.0f, 10.0f, 0.0f };
	g_Rotation = { 0.0f, 0.0f, 0.0f };
	g_Velocity = { 0.0f, 0.0f ,0.0f };
	// g_TargetPos = { g_Pos.x, , 0.0f };
	g_Pitch = 0.0f;
	g_Yaw = 0.0f;
}

void FinalizeRocket()
{
	ModelRelease(g_Model);
}

void UpdateRocket()
{
	const float deltaTime = 1.0f / 60.0f;

	// 限りなく90度に近い数値を取得(90 * 0.99)
	const float pitchLimit = XM_PIDIV2 * 0.99f;

	if (Keyboard_IsKeyDown(KK_A))
	{
		// 左回転
		g_Yaw -= 0.05f;
	}
	else if (Keyboard_IsKeyDown(KK_D))
	{
		// 右回転
		g_Yaw += 0.05f;
	}
	if (Keyboard_IsKeyDown(KK_W))
	{
		// 上回転
		g_Pitch += 0.05f;
	}
	else if (Keyboard_IsKeyDown(KK_S))
	{
		// 下回転
		g_Pitch -= 0.05f;
	}

	// ピッチを制限
	if (g_Pitch > pitchLimit) g_Pitch = pitchLimit; else if (g_Pitch < -pitchLimit) g_Pitch = -pitchLimit;  // 最大値・最小値制限

	// 方向ベクトル取得
	const XMVECTOR forwardBase = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);

	// 進むベクトルを取得
	XMVECTOR goVec = XMVectorZero();

	XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(g_Pitch, g_Yaw, 0.0f);
	XMVECTOR forwardVec = XMVector3TransformNormal(forwardBase, rotationMatrix);

	// goVecに加算
	goVec = XMVectorAdd(goVec, forwardVec);

	// 正規化
	goVec = XMVector3Normalize(goVec);

	// ベクトルの長さを調整
	goVec = XMVectorScale(goVec, 0.2f);

	// 自分のベクトル取得
	XMVECTOR posVec = XMLoadFloat3(&g_Pos);

	// 自分のベクトルに加算
	goVec = XMVectorAdd(posVec, goVec);
	XMStoreFloat3(&g_Pos, goVec);

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