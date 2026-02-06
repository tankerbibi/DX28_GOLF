#include "directx.h"
#include "camera.h"

#include <cmath>

#include "ball.h"
#include "mouse.h"
#include "Keyboard.h"
#include "rocket.h"
#include "easing.h"

static XMMATRIX g_ViewMatrix;
static XMMATRIX g_ProjectionMatrix;

static XMFLOAT3 g_Position;
static XMFLOAT3 g_CameraTargetPos;
static XMFLOAT3 g_CameraRotation;

static float g_CameraYaw = 0.0f;
static float g_CameraPitch = 0.0f;

static CameraMode g_CameraMode = CameraMode::BALL;

static XMFLOAT3 g_FixCameraPosition[3]
{
	{-0.5f, 5.0f, 0.0f},
	{5.0f, 5.0f, 5.0f},
	{ 10.0f, 10.0f, 10.0f}
};

static int g_FixCameraIndex;
static float g_FixCameraTime;
static float g_ShakeTime;
static float g_Shake;
static XMFLOAT3 g_FixCameraOldPosition;

void FollowBall();
void FollowRocket();
void LookBall();

void ReflectsDebugKeyOperations();

void InitializeCamera()
{
	g_Position = { 0.0f, 1.0f, -10.0f };
	g_CameraTargetPos = { 0.0f, 0.0f, 1.0f };
	g_CameraRotation = {0.0f, 0.0f,0.0f};
	g_CameraYaw = 0.0f;
	g_CameraPitch = 0.0f;
	g_CameraMode = CameraMode::BALL;
	g_FixCameraIndex = 0;
	g_FixCameraTime = 0;
	g_ShakeTime = 0;
	g_Shake = 0.0f;
	g_FixCameraOldPosition = { 0.0f, 0.0f, 0.0f };
}

void FinalizeCamera()
{
}

void UpdateCamera()
{
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

	g_FixCameraTime += 1.0f / 60.0f;
	if (g_FixCameraTime > 1.0f)
	{
		g_FixCameraTime = 1.0f;
	}

	if (Keyboard_IsKeyTrigger(KK_D4))
	{
		g_FixCameraIndex = (g_FixCameraIndex + 1) % 3;
		g_FixCameraTime = 0.0f;
		g_FixCameraOldPosition = g_Position;
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
	case CameraMode::LOOKBALL:
		LookBall();
		break;
	case CameraMode::DEBUG:
		ReflectsDebugKeyOperations();
		break;
	default:
		break;
	}

	////// イージング計算
	//float ease = easeInOutCubic(g_FixCameraTime);

	//// 線形補間
	//g_Position.x = g_FixCameraOldPosition.x * (1.0f - ease)
	//	+ g_FixCameraPosition[g_FixCameraIndex].x * ease;
	//g_Position.y = g_FixCameraOldPosition.y * (1.0f - ease)
	//	+ g_FixCameraPosition[g_FixCameraIndex].y * ease;
	//g_Position.z = g_FixCameraOldPosition.z * (1.0f - ease)
	//	+ g_FixCameraPosition[g_FixCameraIndex].z * ease;

	//g_ShakeTime += 1.0f / 60.0f;
	//if (g_ShakeTime > XM_2PI) g_ShakeTime = 0.0f;

	//g_Shake -= 0.1f;
	//if (g_Shake < 0.0f) g_Shake = 0.0f;

	//g_CameraTargetPos.y += sinf(g_ShakeTime * 90.0f) * 0.1f * g_Shake;
}

void DrawCamera()
{
	const XMFLOAT3 up = XMFLOAT3(0.0f, 1.0f, 0.0f);  // カメラの上方向
	g_ViewMatrix = XMMatrixLookAtLH(XMLoadFloat3(&g_Position), XMLoadFloat3(&g_CameraTargetPos), XMLoadFloat3(&up));

	//プロジェクションマトリクス 3dの画面に大きさを合わせるためのマトリクス （新規）
	g_ProjectionMatrix = XMMatrixPerspectiveFovLH(1.3 /*カメラの視野（ラジアン角）*/, screenWidth / screenHeight/*アスペクト比*/, 0.1f/*見ることができる一番近い距離*/, 50000.0f/*見ることができる一番遠い距離*/);
}

void DrawCameraMap()
{
	const XMFLOAT3 position = XMFLOAT3(0.0f, 30.0f, 0.0f);
	const XMFLOAT3 target = XMFLOAT3(0.0f, 0.0f, 0.0f);
	const XMFLOAT3 up = XMFLOAT3(0.0f, 0.0f, 1.0f);  // カメラの上方向 いつもと違うよ！
	g_ViewMatrix = XMMatrixLookAtLH(XMLoadFloat3(&position), XMLoadFloat3(&target), XMLoadFloat3(&up));

	//プロジェクションマトリクス 3dの画面に大きさを合わせるためのマトリクス （新規）
	g_ProjectionMatrix = XMMatrixPerspectiveFovLH(1.3 /*カメラの視野（ラジアン角）*/, 256.0f / 256.0f/*アスペクト比*/, 0.1f/*見ることができる一番近い距離*/, 500.0f/*見ることができる一番遠い距離*/);
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
	forward.x = g_CameraTargetPos.x - g_Position.x;
	forward.y = g_CameraTargetPos.y - g_Position.y;
	forward.z = g_CameraTargetPos.z - g_Position.z;
	return forward;
}

XMFLOAT3 GetCameraPosition()
{
	return g_Position;
}

void SetCameraMode(CameraMode newCameraMode)
{
	g_CameraMode = newCameraMode;
}

CameraMode GetCameraMode()
{
	return g_CameraMode;
	int d;
}

void FollowBall()
{
	XMFLOAT3 ballPos = GetBallPosition();

	g_Position = ballPos;
	//g_Position.z -= 50.0f;
	//g_Position.y += 30.0f;
	g_Position.z -= 10.0f;
	g_Position.y += 10.0f;


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

	/*g_Position.x = g_CameraTargetPos.x + sinf(g_CameraRotation.y) * 3.0f;
	g_Position.z = g_CameraTargetPos.z - cosf(g_CameraRotation.y) * 3.0f;*/
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
	g_Position.x += (newCameraPos.x - g_Position.x) * 0.1f;
	g_Position.y += (newCameraPos.y - g_Position.y) * 0.1f;
	g_Position.z += (newCameraPos.z - g_Position.z) * 0.1f;

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

	XMVECTOR cameraPosVec = XMLoadFloat3(&g_Position);
	velocity = XMVectorAdd(cameraPosVec, velocity);  //
	XMStoreFloat3(&g_Position, velocity);

	XMVECTOR targetPosVec = XMVectorAdd(XMLoadFloat3(&g_Position), forwardVec_WS);  // 注視点ベクトルを導き出す
	XMStoreFloat3(&g_CameraTargetPos, targetPosVec);  // 注視点ベクトルを座標に変換
}

void LookBall()
{
	XMFLOAT3 ballPos = GetBallPosition();

	g_CameraTargetPos.x += (ballPos.x - g_CameraTargetPos.x) * 0.3f;
	g_CameraTargetPos.y += (ballPos.y - g_CameraTargetPos.y) * 0.3f;
	g_CameraTargetPos.z += (ballPos.z - g_CameraTargetPos.z) * 0.3f;
}

void SetCameraShake(float shake)
{
	g_Shake = shake;
	g_ShakeTime = 0.0f;
}
