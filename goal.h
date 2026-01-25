#pragma once

void InitializeGoal();
void FinalizeGoal();
void UpdateGoal();
void DrawGoal();

XMFLOAT3 GetGoalPosition();

void SetGoalPosition(XMFLOAT3 position);

void SetGoalFlagPosition(XMFLOAT3 position);

void SetGoalShake(float Shake);
