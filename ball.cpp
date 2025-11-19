#include "ball.h"
#include "directX.h"
#include "Keyboard.h"
#include "model.h"
#include "camera.h"
#include "shader.h"

static MODEL* g_Model = nullptr;

static XMFLOAT3 g_BallPos;
static XMFLOAT3 g_BallRotation;

void InitializeBall()
{
	g_Model = ModelLoad("asset\\model\\Ball.fbx");
	g_BallPos = {0.0f, 1.75f, 0.0f};
	// g_BallPos = XMFLOAT3(0.0f, 0.0f, 0.0f);  何が違う？
	g_BallRotation = { 0.0f, 0.0f, 0.0f };
}

void FinalizeBall()
{
	ModelRelease(g_Model);
}

void UpdateBall()
{
	g_BallRotation.y += 0.01f;
}

void DrawBall()
{
	Shader_Begin();  // シェーダーの設定
	// 頂点シェーダーに変換行列を設定

	MATRIX matrix;

	matrix.matrix = XMMatrixIdentity();  // 行列を作成　float 4 x 4
	matrix.matrixWorld = XMMatrixIdentity();  // 行列を作成　float 4 x 4

	matrix.matrixWorld *= XMMatrixScaling(1.0f, 1.0f, 1.0f);  // 拡大縮小マトリクス
	matrix.matrixWorld *= XMMatrixRotationRollPitchYaw(g_BallRotation.x, g_BallRotation.y, g_BallRotation.z);  // 回転マトリクス
	matrix.matrixWorld *= XMMatrixTranslation(g_BallPos.x, g_BallPos.y, g_BallPos.z);  // 移動マトリクス。gpuで計算されている。

	matrix.matrix = matrix.matrixWorld;

	matrix.matrix *= GetCameraViewMatrix();  // ビューマトリクス
	matrix.matrix *= GetCameraProjectionMatrix();  // プロジェクションマトリクス

	Shader_SetMatrix(matrix);
	ModelDraw(g_Model);
}