#include "directX.h"
#include "ball.h"
#include "Keyboard.h"
#include "model.h"
#include "camera.h"
#include "shader.h"
#include "field.h"

static MODEL* g_Model = nullptr;

static XMFLOAT3 g_Pos;
static XMFLOAT3 g_Velocity;
static XMFLOAT3 g_Rotation;

void BallHitCheck();

void InitializeBall()
{
	g_Model = ModelLoad("asset\\model\\Ball.fbx");
	g_Pos = {0.0f, 10.0f, 0.0f};
	// g_BallPos = XMFLOAT3(0.0f, 0.0f, 0.0f);  何が違う？
	g_Rotation = { 0.0f, 0.0f, 0.0f };
	g_Velocity = { 0.0f, 0.0f ,0.0f };
}

void FinalizeBall()
{
	ModelRelease(g_Model);
}

void UpdateBall()
{
	float deltaTime = 1.0f / 60.0f;

	XMFLOAT3 cameraForward = GetCameraForward();

	cameraForward.y = 0.0f;

	float length = sqrtf(cameraForward.x * cameraForward.x
						+ cameraForward.y * cameraForward.y 
						+ cameraForward.z * cameraForward.z);

	// Normalize 正規化(長さを１にする)
	cameraForward.x /= length;
	cameraForward.y /= length;
	cameraForward.z /= length;
	
	// 加速
	if (Keyboard_IsKeyDown(KK_A))
	{
		// 例のベクトルの回転の公式
		g_Velocity.x -= -cameraForward.z * 10.0f * deltaTime;
		g_Velocity.z -= cameraForward.x * 10.0f * deltaTime;
		
	}
	else if (Keyboard_IsKeyDown(KK_D))
	{
		g_Velocity.x += cameraForward.z * 10.0f * deltaTime;
		g_Velocity.z += -cameraForward.x * 10.0f * deltaTime;
	}
	if (Keyboard_IsKeyDown(KK_W))
	{
		g_Velocity.x += cameraForward.x * 10.0f * deltaTime;
		g_Velocity.z += cameraForward.z * 10.0f * deltaTime;
	}
	else if (Keyboard_IsKeyDown(KK_S))
	{
		g_Velocity.x -= cameraForward.x * 10.0f * deltaTime;
		g_Velocity.z -= cameraForward.z * 10.0f * deltaTime;
	}

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
	BallHitCheck();
}

void DrawBall()
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

XMFLOAT3 GetBallPos()
{
	return g_Pos;
}

void BallHitCheck()
{
	BLOCK* block = GetFieldBlock();
	float blockRadius = 0.5f;
	float ballRadius = 0.2f;

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
				if (block[i].pos.x - blockRadius < g_Pos.x + ballRadius &&
					g_Pos.x - ballRadius < block[i].pos.x + blockRadius)
				{
					if (block[i].pos.x < g_Pos.x)
					{
						// 右
						g_Pos.x = block[i].pos.x + blockRadius + ballRadius;
					}
					else
					{
						// 左
						g_Pos.x = block[i].pos.x - blockRadius - ballRadius;
					}
					g_Velocity.x *= -e;
				}
			}
			// z方向
			else if(block[i].pos.x - blockRadius < g_Pos.x + ballRadius &&
				g_Pos.x < block[i].pos.x + blockRadius)
			{
				if (block[i].pos.z - blockRadius < g_Pos.z + ballRadius &&
					g_Pos.z - ballRadius < block[i].pos.z + blockRadius)
				{
					if (block[i].pos.z < g_Pos.z)
					{
						// 奥
						g_Pos.z = block[i].pos.z + blockRadius + ballRadius;
					}
					else
					{
						// 手前
						g_Pos.z = block[i].pos.z - blockRadius - ballRadius;
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
					if (block[i].pos.y - blockRadius < g_Pos.y + ballRadius &&
						g_Pos.y - ballRadius < block[i].pos.y + blockRadius)
					{
						if (g_Pos.y > block[i].pos.y)
						{
							// 上
							g_Pos.y = block[i].pos.y + blockRadius + ballRadius;

						}
						else
						{
							// 下
							g_Pos.y = block[i].pos.y - blockRadius - ballRadius;
						}
						g_Velocity.y *= -e;
					}
				}
			}
		}
	}
}