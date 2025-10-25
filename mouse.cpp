#include "directX.h"
#include "mouse.h"

#include <windows.h>

POINT g_MousePoint;

XMFLOAT2 g_MousePosDif;

void InitializeMouse()
{
}

void FinalizeMouse()
{
}

void UpdateMouse()
{
	if (GetCursorPos(&g_MousePoint))
	{
		g_MousePosDif.x = g_MousePoint.x - (screenWidth * 0.5f);
		g_MousePosDif.y = g_MousePoint.y - (screenHeight * 0.5f);
		SetCursorPos(static_cast<int>(screenWidth * 0.5f), static_cast<int>(screenHeight * 0.5f));
	}
}

void DrawMouse()
{

}

XMFLOAT2 GetMousePosDif()
{
	return g_MousePosDif;
}