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
	g_Model = ModelLoad("asset\\model\\Bear2.fbx");
	g_BallPos = {5.0f, 7.0f, -5.0f};
	// g_BallPos = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_BallRotation = { -XM_PIDIV2, 0.0f, 0.0f };
}

void FinalizeBall()
{
	ModelRelease(g_Model);
}

void UpdateBall()
{
}

void DrawBall()
{
	Shader_Begin();  // シェーダーの設定
	// 頂点シェーダーに変換行列を設定
	XMMATRIX matrix{ XMMatrixIdentity() };  // 行列を作成　float 4 x 4

	matrix *= XMMatrixScaling(1.0f, 1.0f, 1.0f);  // 拡大縮小マトリクス
	matrix *= XMMatrixRotationRollPitchYaw(g_BallRotation.x, g_BallRotation.y, g_BallRotation.z);  // 回転マトリクス
	matrix *= XMMatrixTranslation(g_BallPos.x, g_BallPos.y, g_BallPos.z);  // 移動マトリクス。gpuで計算されている。

	matrix *= GetCameraViewMatrix();  // ビューマトリクス

	matrix *= GetCameraProjectionMatrix();  // プロジェクションマトリクス

	Shader_SetMatrix(matrix);
	ModelDraw(g_Model);
}