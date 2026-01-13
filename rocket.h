#pragma once

void InitializeRocket();
void FinalizeRocket();
void UpdateRocket();
void DrawRocket();

float GetRocketYaw();

float GetRocketPitch();

XMFLOAT3 GetRocketPos();
void SetRocketStartPosition(XMFLOAT3 newPosition);
