#include "directx.h"
#include "camera.h"
#include "mouse.h"
#include "Keyboard.h"

XMMATRIX g_ViewMatrix;
XMMATRIX g_ProjectionMatrix;
XMFLOAT3 g_CameraPos;
XMFLOAT3 g_CameraTargetPos;

void InitializeCamera()
{
	g_CameraPos = { 0.0f, 10.0f, -10.0f };
	g_CameraTargetPos = { 0.0f, 0.0f, 0.0f };
}

void FinalizeCamera()
{
}

void UpdateCamera()
{
	g_CameraTargetPos.x += GetMousePosDif().x * 0.05f;
	g_CameraTargetPos.y += -GetMousePosDif().y * 0.05f;
	if (Keyboard_IsKeyDown(KK_W))
	{
		
	}
}

void DrawCamera()
{
	// ビューマトリクス （新規）
	XMFLOAT3 pos = g_CameraPos;
	//XMFLOAT3 target = XMFLOAT3(pos.x, 0.0f, pos.z + 20.0f);  // この書き方何だろう？
	XMFLOAT3 target = g_CameraTargetPos;
	XMFLOAT3 up = XMFLOAT3(0.0f, 1.0f, 0.0f);  // カメラの上方向
	g_ViewMatrix = XMMatrixLookAtLH(XMLoadFloat3(&pos), XMLoadFloat3(&target), XMLoadFloat3(&up));

	//プロジェクションマトリクス 3dの画面に大きさを合わせるためのマトリクス （新規）
	g_ProjectionMatrix = XMMatrixPerspectiveFovLH(1.3 /*カメラの視野（ラジアン角）*/, screenWidth / screenHeight/*アスペクト比*/, 1.0f/*見ることができる一番近い距離*/, 1000.0f/*見ることができる一番遠い距離*/);
}

void AddCameraPosX(float x)
{
	g_CameraPos.x += x;
}

void AddCameraPosY(float y)
{
	g_CameraPos.y += y;
}

void AddCameraPosZ(float z)
{
	g_CameraPos.z += z;
}

void AddCameraPos(float x, float y, float z)
{
	g_CameraPos.x += x;
	g_CameraPos.y += y;
	g_CameraPos.z += z;
}

void SetCameraTargetPos(XMFLOAT3 newTargetPos)
{
	g_CameraTargetPos = newTargetPos;
}

XMMATRIX GetCameraViewMatrix()
{
	return g_ViewMatrix;
}

XMMATRIX GetCameraProjectionMatrix()
{
	return g_ProjectionMatrix;
}