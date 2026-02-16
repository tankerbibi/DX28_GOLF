#pragma once

void InitializeMouse();
void FinalizeMouse();
void UpdateMouse();
void DrawMouse();

XMFLOAT2 GetMousePosDif();

void SetMouseFixed(bool fixed);