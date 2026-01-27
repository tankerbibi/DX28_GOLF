#pragma once

void InitializeTrail();
void FinalizeTrail();
void UpdateTrail();
void DrawTrail();

void SetTrailPosition(XMFLOAT3 position);
void ResetTrailPosition(XMFLOAT3 position);

// トレイルエフェクトを開始します。IDを返します。
int StartTrailEffect();
void FinishTrailEffect(int id);
