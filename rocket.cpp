#include "directX.h"
#include "rocket.h"
#include "Keyboard.h"
#include "model.h"
#include "camera.h"
#include "shader.h"
#include "field.h"
#include "goal.h"
#include "main.h"
#include "ball.h"
#include "effect.h"


enum ROCKET_STATE
{
	ROCKET_STATE_START,
	ROCKET_STATE_MOVE,
	ROCKET_STATE_EXPLODED,
};

static MODEL* g_Model = nullptr;

static XMFLOAT3 g_Position;
static XMFLOAT3 g_Velocity;
static XMFLOAT3 g_Rotation;
static XMFLOAT3 g_TargetPos;

static float g_Pitch;
static float g_Yaw;

static constexpr float g_RocketRadius = 0.2f;

static ROCKET_STATE rocketState;
static int stateCount;

static constexpr float explosionMaxPower = 30.0f;
static constexpr float explosionLength = 20.0f;

void RocketHitCheck();
bool RocketIsHit();
void RocketMove();
void PushBallWithRocket();

void InitializeRocket()
{
	g_Model = ModelLoad("asset\\model\\Rocket2.fbx");
	g_Position = { 0.0f, 10.0f, 0.0f };
	g_Rotation = { 0.0f, 0.0f, 0.0f };
	g_Velocity = { 0.0f, 0.0f ,0.0f };
	// g_TargetPos = { g_Pos.x, , 0.0f };
	g_Pitch = 0.0f;
	g_Yaw = 0.0f;
	rocketState = ROCKET_STATE_MOVE;
	stateCount = 0;
}

void FinalizeRocket()
{
	ModelRelease(g_Model);
}

void UpdateRocket()
{
	if (GetCameraMode() == CameraMode::BALL || GetCameraMode() == CameraMode::DEBUG) return;
	switch (rocketState)
	{
	case ROCKET_STATE_START:
		stateCount++;
		if (stateCount > 60)
		{
			rocketState = ROCKET_STATE_MOVE;
			stateCount = 0;
		}
		break;
	case ROCKET_STATE_MOVE:
		// ロケットを動かす。
		RocketMove();
		// ロケットが当たったら爆発させる処理
		if (RocketIsHit())
		{
			rocketState = ROCKET_STATE_EXPLODED;
			SetCameraMode(CameraMode::LOOKBALL);
			PushBallWithRocket();
		}
		break;
	case ROCKET_STATE_EXPLODED:
		stateCount++;
		
		if (stateCount > 120)
		{
			stateCount = 0;
			g_Position = { 0.0f, 10.0f, 0.0f };
			rocketState = ROCKET_STATE_START;
			SetCameraMode(CameraMode::ROCKET);
		}
		break;
	default:
		break;
	}
}

void DrawRocket()
{
	Shader_Begin();  // シェーダーの設定
	// 頂点シェーダーに変換行列を設定

	MATRIX matrix;

	matrix.matrix = XMMatrixIdentity();  // 行列を作成　float 4 x 4
	matrix.matrixWorld = XMMatrixIdentity();  // 行列を作成　float 4 x 4

	matrix.matrixWorld *= XMMatrixScaling(10.0f, 10.0f, 10.0f);  // 拡大縮小マトリクス
	matrix.matrixWorld *= XMMatrixRotationRollPitchYaw(g_Rotation.x, g_Rotation.y, g_Rotation.z);  // 回転マトリクス
	matrix.matrixWorld *= XMMatrixTranslation(g_Position.x, g_Position.y, g_Position.z);  // 移動マトリクス。gpuで計算されている。

	matrix.matrix = matrix.matrixWorld;

	matrix.matrix *= GetCameraViewMatrix();  // ビューマトリクス
	matrix.matrix *= GetCameraProjectionMatrix();  // プロジェクションマトリクス

	Shader_SetMatrix(matrix);
	ModelDraw(g_Model);
}

XMFLOAT3 GetRocketPos()
{
	return g_Position;
}

float GetRocketYaw()
{
	return g_Yaw;
}

float GetRocketPitch()
{
	return g_Pitch;
}

void RocketMove()
{
	const float deltaTime = 1.0f / 60.0f;

	// 限りなく90度に近い数値を取得(90 * 0.99)
	const float pitchLimit = XM_PIDIV2 * 0.99f;

	// --- 角度の入力処理 ---
	if (Keyboard_IsKeyDown(KK_A)) g_Yaw -= 0.05f;      // 左
	else if (Keyboard_IsKeyDown(KK_D)) g_Yaw += 0.05f; // 右

	if (Keyboard_IsKeyDown(KK_W)) g_Pitch += 0.05f;    // 上
	else if (Keyboard_IsKeyDown(KK_S)) g_Pitch -= 0.05f; // 下

	// ピッチを制限
	if (g_Pitch > pitchLimit) g_Pitch = pitchLimit;
	else if (g_Pitch < -pitchLimit) g_Pitch = -pitchLimit;

	// 角度を変数に反映
	g_Rotation.x = g_Pitch;
	g_Rotation.y = g_Yaw;

	// --- 移動ベクトルの計算 ---
	// 方向ベクトル取得 (Z+ が正面と仮定)
	const XMVECTOR forwardBase = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);

	// 現在の角度から回転行列を作成
	XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(g_Pitch, g_Yaw, 0.0f);

	// 正面ベクトルを回転させる
	XMVECTOR forwardVec = XMVector3TransformNormal(forwardBase, rotationMatrix);

	// 正規化して速度（0.2f）を掛ける
	XMVECTOR moveVec = XMVector3Normalize(forwardVec);
	moveVec = XMVectorScale(moveVec, 0.1f); // 0.2f は移動スピード

	// --- 座標更新 ---
	XMVECTOR posVec = XMLoadFloat3(&g_Position);
	posVec = XMVectorAdd(posVec, moveVec);
	XMStoreFloat3(&g_Position, posVec);
}

// ロケットがブロックに当たったらロールバックする関数
void RocketHitCheck()
{
	BLOCK* block = GetFieldBlock();
	float blockRadius = 0.5f;


	float e = 0.5f;  // 跳ね返り係数

	for (int i = 0; i < blockMax; i++)
	{
		// 横方向の当たり判定処理
		if (block[i].pos.y - blockRadius < g_Position.y &&
			g_Position.y < block[i].pos.y + blockRadius)  // 横からみた図の状況を作り出している！！
		{
			// x方向
			if (block[i].pos.z - blockRadius < g_Position.z &&
				g_Position.z < block[i].pos.z + blockRadius)  // 3次元だから2次元に絞ろう！
			{
				if (block[i].pos.x - blockRadius < g_Position.x + g_RocketRadius &&
					g_Position.x - g_RocketRadius < block[i].pos.x + blockRadius)
				{
					if (block[i].pos.x < g_Position.x)
					{
						// 右
						g_Position.x = block[i].pos.x + blockRadius + g_RocketRadius;
					}
					else
					{
						// 左
						g_Position.x = block[i].pos.x - blockRadius - g_RocketRadius;
					}
					g_Velocity.x *= -e;
				}
			}
			// z方向
			else if (block[i].pos.x - blockRadius < g_Position.x + g_RocketRadius &&
				g_Position.x < block[i].pos.x + blockRadius)
			{
				if (block[i].pos.z - blockRadius < g_Position.z + g_RocketRadius &&
					g_Position.z - g_RocketRadius < block[i].pos.z + blockRadius)
				{
					if (block[i].pos.z < g_Position.z)
					{
						// 奥
						g_Position.z = block[i].pos.z + blockRadius + g_RocketRadius;
					}
					else
					{
						// 手前
						g_Position.z = block[i].pos.z - blockRadius - g_RocketRadius;
					}
				}
			}
		}
		else
			// 縦方向の当たり判定処理
		{
			// 手前　奥
			if (block[i].pos.z - blockRadius < g_Position.z &&
				g_Position.z < block[i].pos.z + blockRadius)
			{
				if (block[i].pos.x - blockRadius < g_Position.x &&
					g_Position.x < block[i].pos.x + blockRadius)
				{
					if (block[i].pos.y - blockRadius < g_Position.y + g_RocketRadius &&
						g_Position.y - g_RocketRadius < block[i].pos.y + blockRadius)
					{
						if (g_Position.y > block[i].pos.y)
						{
							// 上
							g_Position.y = block[i].pos.y + blockRadius + g_RocketRadius;

						}
						else
						{
							// 下
							g_Position.y = block[i].pos.y - blockRadius - g_RocketRadius;
						}
						g_Velocity.y *= -e;
					}
				}
			}
		}
	}
}

// ロケットがブロックに当たったらtrueを返す関数
bool RocketIsHit()
{
	BLOCK* block = GetFieldBlock();
	float blockRadius = 0.5f;


	float e = 0.5f;  // 跳ね返り係数

	for (int i = 0; i < blockMax; i++)
	{
		// 横方向の当たり判定処理
		if (block[i].pos.y - blockRadius < g_Position.y &&
			g_Position.y < block[i].pos.y + blockRadius)  // 横からみた図の状況を作り出している！！
		{
			// x方向
			if (block[i].pos.z - blockRadius < g_Position.z &&
				g_Position.z < block[i].pos.z + blockRadius)  // 3次元だから2次元に絞ろう！
			{
				if (block[i].pos.x - blockRadius < g_Position.x + g_RocketRadius &&
					g_Position.x - g_RocketRadius < block[i].pos.x + blockRadius)
				{
					if (block[i].pos.x < g_Position.x)
					{
						// 右
						g_Position.x = block[i].pos.x + blockRadius + g_RocketRadius;
						return true;
					}
					else
					{
						// 左
						g_Position.x = block[i].pos.x - blockRadius - g_RocketRadius;
						return true;
					}
					g_Velocity.x *= -e;
				}
			}
			// z方向
			else if (block[i].pos.x - blockRadius < g_Position.x + g_RocketRadius &&
				g_Position.x < block[i].pos.x + blockRadius)
			{
				if (block[i].pos.z - blockRadius < g_Position.z + g_RocketRadius &&
					g_Position.z - g_RocketRadius < block[i].pos.z + blockRadius)
				{
					if (block[i].pos.z < g_Position.z)
					{
						// 奥
						g_Position.z = block[i].pos.z + blockRadius + g_RocketRadius;
						return true;
					}
					else
					{
						// 手前
						g_Position.z = block[i].pos.z - blockRadius - g_RocketRadius;
						return true;
					}
				}
			}
		}
		else
			// 縦方向の当たり判定処理
		{
			// 手前　奥
			if (block[i].pos.z - blockRadius < g_Position.z &&
				g_Position.z < block[i].pos.z + blockRadius)
			{
				if (block[i].pos.x - blockRadius < g_Position.x &&
					g_Position.x < block[i].pos.x + blockRadius)
				{
					if (block[i].pos.y - blockRadius < g_Position.y + g_RocketRadius &&
						g_Position.y - g_RocketRadius < block[i].pos.y + blockRadius)
					{
						if (g_Position.y > block[i].pos.y)
						{
							// 上
							g_Position.y = block[i].pos.y + blockRadius + g_RocketRadius;
							return true;

						}
						else
						{
							// 下
							g_Position.y = block[i].pos.y - blockRadius - g_RocketRadius;
							return true;
						}
						g_Velocity.y *= -e;
					}
				}
			}
		}
	}
	return false;
}

void PushBallWithRocket()
{
	XMFLOAT3 ballPosition = GetBallPosition();
	XMFLOAT3 force;

	force.x = ballPosition.x - g_Position.x;
	force.y = ballPosition.y - g_Position.y;
	force.z = ballPosition.z - g_Position.z;

	float length = sqrtf((ballPosition.x - g_Position.x) * (ballPosition.x - g_Position.x)
		+ (ballPosition.y - g_Position.y) * (ballPosition.y - g_Position.y)
		+ (ballPosition.z - g_Position.z) * (ballPosition.z - g_Position.z));

	float power = explosionMaxPower * (1.0f - (length / 20.0f));

	if (length > 1.0f)
	{
		force.x /= length;
		force.y /= length;
		force.z /= length;
	}

	if (length <= 20.0f)
	{
		// ボールに力を加える。
		AddForce({ force.x * power, force.y * power + 5.0f, force.z * power});
		CreateEffect(g_Position);
	}

}
