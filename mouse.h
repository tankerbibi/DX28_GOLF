#pragma once

enum class MOUSE_BUTTON {
    LEFT,
    RIGHT,
    MIDDLE,
    MAX_BUTTONS
};

void InitializeMouse();
void FinalizeMouse();
void UpdateMouse();
void DrawMouse();

XMFLOAT2 GetMousePosDif();

void SetMouseFixed(bool fixed);

bool IsMouseTriggered(MOUSE_BUTTON button);
bool IsMousePressed(MOUSE_BUTTON button);

XMFLOAT2 GetMousePosition();
