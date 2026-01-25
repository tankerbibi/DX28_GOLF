#pragma once

void InitializeBall();
void FinalizeBall();
void UpdateBall();
void DrawBall();

XMFLOAT3 GetBallPosition();
void AddForce(XMFLOAT3 force);
