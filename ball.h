#pragma once
#include <DirectXMath.h>

using namespace DirectX;

void InitializeBall();
void FinalizeBall();
void UpdateBall();
void DrawBall();

XMFLOAT3 GetBallPosition();
void AddForce(XMFLOAT3 force);
