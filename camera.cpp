#include "directx.h"
#include "camera.h"

#include <cmath>


#include "mouse.h"
#include "Keyboard.h"

static XMMATRIX g_ViewMatrix;
static XMMATRIX g_ProjectionMatrix;
static XMFLOAT3 g_CameraPos;
static XMFLOAT3 g_CameraTargetPos;

static float g_CameraYaw = 0.0f;
static float g_CameraPitch = 0.0f;

void InitializeCamera()
{
	g_CameraPos = { 0.0f, 10.0f, -10.0f };
	g_CameraTargetPos = { 0.0f, 0.0f, 1.0f };
}

void FinalizeCamera()
{
}

void UpdateCamera()
{
	XMFLOAT2 mousePosDif = GetMousePosDif();

	g_CameraYaw += mousePosDif.x * 0.005f;
	g_CameraPitch += mousePosDif.y * 0.005f;

	const float pitchLimit = XM_PIDIV2 * 0.99f;
	if (g_CameraPitch > pitchLimit) g_CameraPitch = pitchLimit; if (g_CameraPitch < -pitchLimit) g_CameraPitch = -pitchLimit;  // 最大値・最小値制限

	XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(g_CameraPitch, g_CameraYaw, 0.0f);

	const XMVECTOR forwardBase = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f); 

	XMVECTOR forwardVec = XMVector3TransformCoord(forwardBase, rotationMatrix);

	XMVECTOR cameraPosVec = XMLoadFloat3(&g_CameraPos);
	XMVECTOR targetPosVec = cameraPosVec + forwardVec;

	XMStoreFloat3(&g_CameraTargetPos, targetPosVec);  // 計算結果をg_CameraTargetPosに保存。

	if (Keyboard_IsKeyDown(KK_W))
	{
		XMStoreFloat3(&g_CameraPos, targetPosVec);
	}
	else if (Keyboard_IsKeyDown(KK_S))
	{
		XMStoreFloat3(&g_CameraPos, XMVectorNegate(targetPosVec));
	}
	if (Keyboard_IsKeyDown(KK_D))
	{
		XMMATRIX leftRotationMatrix = XMMatrixRotationRollPitchYaw(0, -90.0f, 0);
		XMVECTOR leftVec = XMVector3TransformCoord(forwardBase, leftRotationMatrix);
		XMVECTOR posVec = cameraPosVec + leftVec;
		XMStoreFloat3(&g_CameraPos, posVec);
	}
	else if (Keyboard_IsKeyDown(KK_A))
	{
		XMMATRIX rightRotationMatrix = XMMatrixRotationRollPitchYaw(0, 90.0f, 0);
		XMVECTOR rightVec = XMVector3TransformCoord(forwardBase, rightRotationMatrix);
		XMVECTOR posVec = cameraPosVec + rightVec;
		XMStoreFloat3(&g_CameraPos, posVec);
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