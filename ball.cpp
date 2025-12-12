#include "directX.h"
#include "ball.h"
#include "Keyboard.h"
#include "model.h"
#include "camera.h"
#include "shader.h"
#include "field.h"
#include "goal.h"
#include "main.h"
#include "stroke.h"
#include "ranking.h"
#include "effect.h"
#include "trail.h"

static MODEL* g_Model = nullptr;

static XMFLOAT3 g_Position;
static XMFLOAT3 g_Velocity;
static XMFLOAT3 g_Rotation;
// 外部的な力
static XMFLOAT3 g_OutForce;

enum BALL_STATE
{
	BALL_STATE_MOVE,
	BALL_STATE_GOAL,
};
static BALL_STATE g_State;
static int g_StateCount;

static constexpr float g_BallRadius = 0.5f;

void BallHitCheck();
void MoveBall();

void InitializeBall()
{
	g_Model = ModelLoad("asset\\model\\Ball.fbx");
	g_Position = {0.0f, 10.0f, 0.0f};
	// g_BallPos = XMFLOAT3(0.0f, 0.0f, 0.0f);  何が違う？
	g_Rotation = { 0.0f, 0.0f, 0.0f };
	g_Velocity = { 0.0f, 0.0f ,0.0f };

	g_State = BALL_STATE_MOVE;
	g_StateCount = 0;
}

void FinalizeBall()
{
	ModelRelease(g_Model);
}

void UpdateBall()
{
	// ステートマシン ステートパターンというものもあるらしい。
	switch (g_State)
	{
	case BALL_STATE_MOVE:
		MoveBall();
		break;
	case BALL_STATE_GOAL:
		g_StateCount++;
		if (g_StateCount > 60)
		{
			SetScene(SCENE_RESULT);
		}
		break;
	default:
		break;
	}
}
void MoveBall()
{
	XMFLOAT3 cameraForward = GetCameraForward();

	// y成分を消す
	cameraForward.y = 0.0f;

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

	// 加速
	CameraMode cameraMode = GetCameraMode();
	if (cameraMode == CameraMode::BALL)
	{
		if (Keyboard_IsKeyDown(KK_A))
		{
			// 例のベクトルの回転の公式
			force.x -= cameraForward.z; // xとzを返ればいいだけ
			force.z += cameraForward.x;

		}
		else if (Keyboard_IsKeyDown(KK_D))
		{
			force.x += cameraForward.z;
			force.z -= cameraForward.x;
		}
		if (Keyboard_IsKeyDown(KK_W))
		{
			force.x += cameraForward.x;
			force.z += cameraForward.z;
		}
		else if (Keyboard_IsKeyDown(KK_S))
		{
			force.x -= cameraForward.x;
			force.z -= cameraForward.z;
		}
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
		g_Velocity.x += cameraForward.x * 5.0f;
		g_Velocity.z += cameraForward.z * 5.0f;

		g_Velocity.y += 5.0f;  // 撃力

		// 打数加算
		AddStroke(1);
		
		SetRankingScore(GetStroke());

		CreateEffect(g_Position);
	}

	// 重力
	g_Velocity.y -= 9.8f * deltaTime;

	// 抵抗
	g_Velocity.x -= g_Velocity.x * 2.0f * deltaTime;
	g_Velocity.y -= g_Velocity.y * 0.5f * deltaTime;
	g_Velocity.z -= g_Velocity.z * 2.0f * deltaTime;

	// 移動
	g_Position.x += g_Velocity.x * deltaTime;
	g_Position.y += g_Velocity.y * deltaTime;
	g_Position.z += g_Velocity.z * deltaTime;

	// 衝突判定
	BallHitCheck();

	SetTrailPosition(g_Position);

	// 終点から始点を引くことで、ベクトルを求める
	XMFLOAT3 goalVec;
	XMFLOAT3 goalPosition = GetGoalPosition();
	goalVec.x = goalPosition.x - g_Position.x;
	goalVec.y = goalPosition.y - g_Position.y;
	goalVec.z = goalPosition.z - g_Position.z;

	float goalLength = sqrtf(goalVec.x * goalVec.x
		+ goalVec.y * goalVec.y
		+ goalVec.z * goalVec.z);

	// ゴール衝突判定
	if (goalLength < g_BallRadius * 4.0f)
	{
		SetRankingScore(GetStroke());

		g_State = BALL_STATE_GOAL;
		g_StateCount = 0;
	}
}

void DrawBall()
{
	Shader_Begin();  // シェーダーの設定
	// 頂点シェーダーに変換行列を設定

	MATRIX matrix;

	matrix.matrix = XMMatrixIdentity();  // 行列を作成　float 4 x 4
	matrix.matrixWorld = XMMatrixIdentity();  // 行列を作成　float 4 x 4

	matrix.matrixWorld *= XMMatrixScaling(5.0f, 5.0f, 5.0f);  // 拡大縮小マトリクス
	matrix.matrixWorld *= XMMatrixRotationRollPitchYaw(g_Rotation.x, g_Rotation.y, g_Rotation.z);  // 回転マトリクス
	matrix.matrixWorld *= XMMatrixTranslation(g_Position.x, g_Position.y, g_Position.z);  // 移動マトリクス。gpuで計算されている。

	matrix.matrix = matrix.matrixWorld;

	matrix.matrix *= GetCameraViewMatrix();  // ビューマトリクス
	matrix.matrix *= GetCameraProjectionMatrix();  // プロジェクションマトリクス

	Shader_SetMatrix(matrix);
	ModelDraw(g_Model);
}

XMFLOAT3 GetBallPos()
{
	return g_Position;
}

void BallHitCheck()
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
				if (block[i].pos.x - blockRadius < g_Position.x + g_BallRadius &&
					g_Position.x - g_BallRadius < block[i].pos.x + blockRadius)
				{
					if (block[i].pos.x < g_Position.x)
					{
						// 右
						g_Position.x = block[i].pos.x + blockRadius + g_BallRadius;
					}
					else
					{
						// 左
						g_Position.x = block[i].pos.x - blockRadius - g_BallRadius;
					}
					g_Velocity.x *= -e;
				}
			}
			// z方向
			else if(block[i].pos.x - blockRadius < g_Position.x + g_BallRadius &&
				g_Position.x < block[i].pos.x + blockRadius)
			{
				if (block[i].pos.z - blockRadius < g_Position.z + g_BallRadius &&
					g_Position.z - g_BallRadius < block[i].pos.z + blockRadius)
				{
					if (block[i].pos.z < g_Position.z)
					{
						// 奥
						g_Position.z = block[i].pos.z + blockRadius + g_BallRadius;
					}
					else
					{
						// 手前
						g_Position.z = block[i].pos.z - blockRadius - g_BallRadius;
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
					if (block[i].pos.y - blockRadius < g_Position.y + g_BallRadius &&
						g_Position.y - g_BallRadius < block[i].pos.y + blockRadius)
					{
						if (g_Position.y > block[i].pos.y)
						{
							// 上
							g_Position.y = block[i].pos.y + blockRadius + g_BallRadius;
							
							if (g_Velocity.y < -3.0f)
							{
								CreateEffect(g_Position);
							}
						}
						else
						{
							// 下
							g_Position.y = block[i].pos.y - blockRadius - g_BallRadius;
						}
						g_Velocity.y *= -e;
					}
				}
			}
		}
	}
}

void AddForce(XMFLOAT3 force)
{
	g_Velocity.x += force.x;
	g_Velocity.y += force.y;
	g_Velocity.z += force.z;
}