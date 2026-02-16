#include "directX.h"
#include "mouse.h"

#include <windows.h>

bool g_MouseFixed = false;

POINT g_MousePoint;
XMFLOAT2 g_MousePosDif;

void InitializeMouse()
{
	g_MouseFixed = false;
}

void FinalizeMouse()
{
}

void UpdateMouse()
{
	if (g_MouseFixed)
	{
		if (GetCursorPos(&g_MousePoint))
		{
			g_MousePosDif.x = g_MousePoint.x - (screenWidth * 0.5f);
			g_MousePosDif.y = g_MousePoint.y - (screenHeight * 0.5f);

			SetCursorPos(
				static_cast<int>(screenWidth * 0.5f),
				static_cast<int>(screenHeight * 0.5f)
			);
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
