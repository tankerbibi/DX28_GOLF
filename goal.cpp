#include "directX.h"
#include "goal.h"
#include "Keyboard.h"
#include "model.h"
#include "camera.h"
#include "shader.h"
#include "field.h"

static MODEL* g_Model = nullptr;

static XMFLOAT3 g_Position;
static XMFLOAT3 g_Velocity;
static XMFLOAT3 g_Rotation;


void InitializeGoal()
{
	g_Model = ModelLoad("asset\\model\\Goal.fbx");
	g_Position = { 0.0f, 2.0f, 9.0f };
	g_Rotation = { 0.0f, 0.0f, 0.0f };
}

void FinalizeGoal()
{
	ModelRelease(g_Model);
}

void UpdateGoal()
{

}

void DrawGoal()
{
	// 頂点シェーダーに変換行列を設定

	MATRIX matrix;

	matrix.matrix = XMMatrixIdentity();  // 行列を作成　float 4 x 4
	matrix.matrixWorld = XMMatrixIdentity();  // 行列を作成　float 4 x 4

	matrix.matrixWorld *= XMMatrixScaling(1.0f, 1.0f, 1.0f);  // 拡大縮小マトリクス
	matrix.matrixWorld *= XMMatrixRotationRollPitchYaw(g_Rotation.x, g_Rotation.y, g_Rotation.z);  // 回転マトリクス
	matrix.matrixWorld *= XMMatrixTranslation(g_Position.x, g_Position.y, g_Position.z);  // 移動マトリクス。gpuで計算されている。

	matrix.matrix = matrix.matrixWorld;

	matrix.matrix *= GetCameraViewMatrix();  // ビューマトリクス
	matrix.matrix *= GetCameraProjectionMatrix();  // プロジェクションマトリクス

	Shader_SetMatrix(matrix);
	ModelDraw(g_Model);
}

XMFLOAT3 GetGoalPosition()
{
	return g_Position;
}

XMFLOAT3 GetGoalPos()
{
	return g_Position;
}