#include "ball.h"
#include "directX.h"
#include "Keyboard.h"
#include "model.h"
#include "camera.h"
#include "shader.h"
#include "field.h"

static MODEL* g_Model = nullptr;

static XMFLOAT3 g_Pos;
static XMFLOAT3 g_Velocity;
static XMFLOAT3 g_Rotation;

void InitializeBall()
{
	g_Model = ModelLoad("asset\\model\\Ball.fbx");
	g_Pos = {0.0f, 1.75f, 0.0f};
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

	if (Keyboard_IsKeyDown(KK_A))
	{
		g_Velocity.x += 10.0f * deltaTime;
	}
	else if (Keyboard_IsKeyDown(KK_D))
	{
		g_Velocity.x -= 10.0f * deltaTime;
	}
	if (Keyboard_IsKeyDown(KK_W))
	{
		g_Velocity.z += 10.0f * deltaTime;
	}
	else if (Keyboard_IsKeyDown(KK_S))
	{
		g_Velocity.z -= 10.0f * deltaTime;
	}

	g_Velocity.y -= 9.8f * deltaTime;

	g_Velocity.x -= g_Velocity.x * 2.0f * deltaTime;
	g_Velocity.y -= g_Velocity.y * 2.0f * deltaTime;
	g_Velocity.z -= g_Velocity.z * 2.0f * deltaTime;

	g_Pos.x += g_Velocity.x * deltaTime;
	g_Pos.y += g_Velocity.y * deltaTime;
	g_Pos.z += g_Velocity.z * deltaTime;
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

void BallHitCheck()
{
	BLOCK* block = GetFieldBlock();

	for (int i = 0; i < blockMax; i++)
	{
		
	}
}