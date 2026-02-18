#include "directX.h"
#include "mouse.h"
#include "main.h"

#include <windows.h>

bool g_MouseFixed = false;

POINT g_MousePoint;
XMFLOAT2 g_MousePosDif;

bool g_CurrentState[static_cast<int>(MOUSE_BUTTON::MAX_BUTTONS)];
bool g_PreviousState[static_cast<int>(MOUSE_BUTTON::MAX_BUTTONS)];
int vKeys[] = { VK_LBUTTON, VK_RBUTTON, VK_MBUTTON };

static constexpr SHORT KEY_PRESSED = 0x8000;
static constexpr float CENTER_RATIO = 0.5f;

void InitializeMouse()
{
	g_MouseFixed = false;
}

void FinalizeMouse()
{
}

void UpdateMouse()
{
	for (int i = 0; i < static_cast<int>(MOUSE_BUTTON::MAX_BUTTONS); i++)
	{
		g_PreviousState[i] = g_CurrentState[i];
		g_CurrentState[i] = (GetAsyncKeyState(vKeys[i]) & KEY_PRESSED) != 0;
	}

	if (g_MouseFixed)
	{
		if (GetCursorPos(&g_MousePoint))
		{
			g_MousePosDif.x = g_MousePoint.x - (screenWidth * CENTER_RATIO);
			g_MousePosDif.y = g_MousePoint.y - (screenHeight * CENTER_RATIO);

			XMFLOAT2 center = { static_cast<int>(screenWidth * CENTER_RATIO), static_cast<int>(screenHeight * CENTER_RATIO) };
			SetCursorPos(center.x, center.y);
		}
	}
	else
	{
		g_MousePosDif.x = 0.0f;
		g_MousePosDif.y = 0.0f;
	}
}

void DrawMouse()
{
}

XMFLOAT2 GetMousePosDif()
{
	return g_MousePosDif;
}

void SetMouseFixed(bool fixed)
{
	g_MouseFixed = fixed;

}

bool IsMouseTriggered(MOUSE_BUTTON button)
{
	return g_CurrentState[static_cast<int>(button)] && !g_PreviousState[static_cast<int>(button)];
}

bool IsMousePressed(MOUSE_BUTTON button)
{
	return g_CurrentState[static_cast<int>(button)];
}

XMFLOAT2 GetMousePosition()
{
	POINT point;
	GetCursorPos(&point); // デスクトップ全体の座標を取得

	// ★ここがポイント：現在アクティブなウィンドウ（自分のゲーム画面）を自動取得
	HWND hWnd = GetWindow();

	// ウィンドウ内の座標に変換
	ScreenToClient(hWnd, &point);

	return XMFLOAT2(static_cast<float>(point.x), static_cast<float>(point.y));
}
