#include "directX.h"

#include "sky.h"
#include "model.h"
#include "shader.h"
#include "camera.h"

static MODEL* g_Model;

static XMFLOAT3 g_Position;
static XMFLOAT3 g_Rotation;

void InitializeSky()
{
	g_Model = ModelLoad("asset\\model\\sky2.fbx");
	g_Position = { 0.0f, 0.0f, 0.0f };
	g_Rotation = { 0.0f, 0.0f, 0.0f };
}

void FinalizeSky()
{
	ModelRelease(g_Model);
}

void UpdateSky()
{
	g_Position = GetCameraPosition();
}

void DrawSky()
{
	MATRIX commonMatrices;
	// íPà çsóÒÇ…èâä˙âª
	commonMatrices.matrixWorld = XMMatrixIdentity();
	commonMatrices.matrix = XMMatrixIdentity();

	commonMatrices.matrixWorld *= XMMatrixScaling(10000.0f, 10000.0f, 10000.0f);	
	commonMatrices.matrixWorld *= XMMatrixRotationRollPitchYaw(g_Rotation.x, g_Rotation.y, g_Rotation.z);
	commonMatrices.matrixWorld *= XMMatrixTranslation(g_Position.x, g_Position.y, g_Position.z);

	commonMatrices.matrix = commonMatrices.matrixWorld;


	commonMatrices.matrix *= GetCameraViewMatrix() * GetCameraProjectionMatrix();
	Shader_SetMatrix(commonMatrices);

	ModelDraw(g_Model);
}
