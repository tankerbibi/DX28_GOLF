#include "directx.h"
#include "camera.h"

#include <cmath>

#include "ball.h"
#include "mouse.h"
#include "Keyboard.h"
#include "rocket.h"

static XMMATRIX g_ViewMatrix;
static XMMATRIX g_ProjectionMatrix;

static XMFLOAT3 g_CameraPosition;
static XMFLOAT3 g_CameraTargetPos;
static XMFLOAT3 g_CameraRotation;

static float g_CameraYaw = 0.0f;
static float g_CameraPitch = 0.0f;

static CameraMode g_CameraMode = CameraMode::BALL;

void FollowBall();
void FollowRocket();
void ReflectsDebugKeyOperations();

void InitializeCamera()
{
	g_CameraPosition = { 0.0f, 1.0f, -10.0f };
	g_CameraTargetPos = { 0.0f, 0.0f, 1.0f };
	g_CameraRotation = {0.0f, 0.0f,0.0f};
	g_CameraYaw = 0.0f;
	g_CameraPitch = 0.0f;
}

void FinalizeCamera()
{
}

void UpdateCamera()
{
	//////////////////////////////////////////////////////////////
	

	//////////////////////////////////////////////////////////////////

	if (Keyboard_IsKeyTrigger(KK_D1))
	{
		g_CameraMode = CameraMode::BALL;
	}
	else if (Keyboard_IsKeyTrigger(KK_D2))
	{
		g_CameraMode = CameraMode::ROCKET;
	}
	else if (Keyboard_IsKeyTrigger(KK_D3))
	{
		g_CameraMode = CameraMode::DEBUG;
	}

	switch (g_CameraMode)
	{
	case CameraMode::PLAY:
		ReflectsDebugKeyOperations();
		break;
	case CameraMode::ROCKET:
		FollowRocket();
		break;
	case CameraMode::BALL:
		FollowBall();
		break;
	case CameraMode::DEBUG:
		ReflectsDebugKeyOperations();
		break;
	default:
		break;
	}

	///授業コード終わり///
}

void DrawCamera()
{
	const XMFLOAT3 up = XMFLOAT3(0.0f, 1.0f, 0.0f);  // カメラの上方向
	g_ViewMatrix = XMMatrixLookAtLH(XMLoadFloat3(&g_CameraPosition), XMLoadFloat3(&g_CameraTargetPos), XMLoadFloat3(&up));

	//プロジェクションマトリクス 3dの画面に大きさを合わせるためのマトリクス （新規）
	g_ProjectionMatrix = XMMatrixPerspectiveFovLH(1.3 /*カメラの視野（ラジアン角）*/, screenWidth / screenHeight/*アスペクト比*/, 1.0f/*見ることができる一番近い距離*/, 1000.0f/*見ることができる一番遠い距離*/);
}

XMMATRIX GetCameraViewMatrix()
{
	return g_ViewMatrix;
}

XMMATRIX GetCameraProjectionMatrix()
{
	return g_ProjectionMatrix;
}

XMFLOAT3 GetCameraForward()
{
	// 終点から始点を引く
	XMFLOAT3 forward;
	forward.x = g_CameraTargetPos.x - g_CameraPosition.x;
	forward.y = g_CameraTargetPos.y - g_CameraPosition.y;
	forward.z = g_CameraTargetPos.z - g_CameraPosition.z;
	return forward;
}

XMFLOAT3 GetCameraPosition()
{
	return g_CameraPosition;
}

CameraMode GetCameraMode()
{
	return g_CameraMode;
	int d;
}

void FollowBall()
{
	XMFLOAT3 ballPos = GetBallPos();

	g_CameraPosition = ballPos;
	g_CameraPosition.z -= 5.0f;
	g_CameraPosition.y += 5.0f;

	g_CameraTargetPos.x += (ballPos.x - g_CameraTargetPos.x) * 0.3f;
	g_CameraTargetPos.y += (ballPos.y - g_CameraTargetPos.y) * 0.3f;
	g_CameraTargetPos.z += (ballPos.z - g_CameraTargetPos.z) * 0.3f;

	///授業コード始まり///
	if (Keyboard_IsKeyDown(KK_RIGHT))
	{
		g_CameraTargetPos.x += 0.1f;
	}
	if (Keyboard_IsKeyDown(KK_LEFT))
	{
		g_CameraTargetPos.x -= 0.1f;
	}

	if (Keyboard_IsKeyDown(KK_K))
	{
		g_CameraRotation.y += 0.1f;
	}
	if (Keyboard_IsKeyDown(KK_J))
	{
		g_CameraRotation.y -= 0.1f;
	}

	g_CameraPosition.x = g_CameraTargetPos.x + sinf(g_CameraRotation.y) * 3.0f;
	g_CameraPosition.z = g_CameraTargetPos.z - cosf(g_CameraRotation.y) * 3.0f;
}

void FollowRocket()
{
	float rocketYaw = GetRocketYaw();
	float rocketPitch = GetRocketPitch();

	// カメラとロケットの距離（オフセット）
	const float distance = 2.0f;
	const float height = 2.0f;

	XMFLOAT3 rocketPos = GetRocketPos();

	// 回転マトリクスを取得
	XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(0.0f, rocketYaw, 0.0f);
	// 後方ベクトルを取得
	XMVECTOR backWard = XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f);
	XMVECTOR offsetVec = XMVector3TransformNormal(backWard, rotationMatrix);

	// ベクトルをオフセット分スケール
	offsetVec = XMVectorScale(offsetVec, distance);

	// ターゲットベクトルを取得
	XMVECTOR targetPosVec = XMLoadFloat3(&rocketPos);
	// ターゲットベクトルにオフセットの情報を追加
	XMVECTOR newCameraPosVec = XMVectorAdd(targetPosVec, offsetVec);

	// Y軸（高さ）を調整
	XMFLOAT3 newCameraPos;
	XMStoreFloat3(&newCameraPos, newCameraPosVec);
	newCameraPos.y += height;
	
	// カメラの位置を補間する
	g_CameraPosition.x += (newCameraPos.x - g_CameraPosition.x) * 0.1f;
	g_CameraPosition.y += (newCameraPos.y - g_CameraPosition.y) * 0.1f;
	g_CameraPosition.z += (newCameraPos.z - g_CameraPosition.z) * 0.1f;

	// 注視点（ターゲット）をロケットの位置の少し上に設定。
	g_CameraTargetPos.x = rocketPos.x;
	g_CameraTargetPos.y = rocketPos.y + 0.3f;
	g_CameraTargetPos.z = rocketPos.z;
}

void ReflectsDebugKeyOperations()
{
	XMFLOAT2 mousePosDif = GetMousePosDif();

	g_CameraYaw += mousePosDif.x * 0.0018f;
	g_CameraPitch += mousePosDif.y * 0.0018f;

	const float pitchLimit = XM_PIDIV2 * 0.99f;  // 限りなく90度に近い数値を取得(90 * 0.99)
		
	if (g_CameraPitch > pitchLimit) g_CameraPitch = pitchLimit; else if (g_CameraPitch < -pitchLimit) g_CameraPitch = -pitchLimit;  // 最大値・最小値制限

	const XMVECTOR forwardBase = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);  // 方向ベクトル取得
	// --- 1. W/S (前後) 用の前方ベクトルを計算 (ピッチを考慮する) ---
	XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(g_CameraPitch, g_CameraYaw, 0.0f);
	XMVECTOR forwardVec_WS = XMVector3TransformNormal(forwardBase, rotationMatrix);

	// --- 2. A/D (左右) 用の"水平"前方ベクトルを計算 (ピッチを 0.0f にする) ---
	XMMATRIX horizontalRotationMatrix = XMMatrixRotationRollPitchYaw(0.0f, g_CameraYaw, 0.0f);
	XMVECTOR forwardVec_AD = XMVector3TransformNormal(forwardBase, horizontalRotationMatrix);

	XMVECTOR velocity = XMVectorZero();

	if (Keyboard_IsKeyDown(KK_UP))
	{
		velocity = XMVectorAdd(velocity, forwardVec_WS);
	}
	else if (Keyboard_IsKeyDown(KK_DOWN))
	{
		velocity = XMVectorAdd(velocity, XMVectorNegate(forwardVec_WS));
	}

	if (Keyboard_IsKeyDown(KK_LEFT))
	{
		XMMATRIX leftRotationMatrix = XMMatrixRotationRollPitchYaw(0.0f, -XM_PIDIV2, 0);  // 回転マトリクスを取得
		XMVECTOR left = XMVector3TransformNormal(forwardVec_AD, leftRotationMatrix);  // 前方ベクトルを回転
		velocity = XMVectorAdd(velocity, left);
	}
	else if (Keyboard_IsKeyDown(KK_RIGHT))
	{
		XMMATRIX rightRotationMatrix = XMMatrixRotationRollPitchYaw(0.0f, XM_PIDIV2, 0);  // 回転マトリクスを取得
		XMVECTOR right = XMVector3TransformNormal(forwardVec_AD, rightRotationMatrix);  // 前方ベクトルを回転
		velocity = XMVectorAdd(velocity, right);
	}

	if (Keyboard_IsKeyDown(KK_E))
	{
		XMMATRIX upRotationMatrix = XMMatrixRotationRollPitchYaw(-XM_PIDIV2, 0, 0);  // 回転マトリクスを取得
		XMVECTOR up = XMVector3TransformNormal(forwardBase, upRotationMatrix);  // 前方ベクトルを回転
		velocity = XMVectorAdd(velocity, up);
	}
	else if (Keyboard_IsKeyDown(KK_Q))
	{
		XMMATRIX downRotationMatrix = XMMatrixRotationRollPitchYaw(XM_PIDIV2, 0, 0);  // 回転マトリクスを取得
		XMVECTOR down = XMVector3TransformNormal(forwardBase, downRotationMatrix);  // 前方ベクトルを回転
		velocity = XMVectorAdd(velocity, down);
	}

	velocity = XMVector3Normalize(velocity);

	velocity = XMVectorScale(velocity, 0.2f);

	XMVECTOR cameraPosVec = XMLoadFloat3(&g_CameraPosition);
	velocity = XMVectorAdd(cameraPosVec, velocity);  //
	XMStoreFloat3(&g_CameraPosition, velocity);

	XMVECTOR targetPosVec = XMVectorAdd(XMLoadFloat3(&g_CameraPosition), forwardVec_WS);  // 注視点ベクトルを導き出す
	XMStoreFloat3(&g_CameraTargetPos, targetPosVec);  // 注視点ベクトルを座標に変換
}
