#include "directX.h"
#include "player.h"
#include "mouse.h"
#include "rocket.h"
#include "camera.h"
#include "bomb.h"
#include "Keyboard.h"

static constexpr float bombPositionGapY = 0.5f;

void InitializePlayer()
{
}

void FinalizePlayer()
{
}

void UpdatePlayer()
{
	if (Keyboard_IsKeyDown(KK_W))
	{

	}
	if (IsMouseTriggered(MOUSE_BUTTON::LEFT))
	{
		// 4. 発射！（カメラの位置から、カメラの正面方向へ）
		XMFLOAT3 cameraForward = GetCameraForward();
		XMFLOAT3 cameraPosition = GetCameraPosition();
		CreateBomb({cameraPosition.x, cameraPosition.y - bombPositionGapY , cameraPosition.z }, { cameraForward.x * 2000.0f, cameraForward.y * 2000.0f, cameraForward.z * 2000.0f });
		// CreateRocket(GetCameraPosition(), cameraForward);

	}
}

void DrawPlayer()
{
}
