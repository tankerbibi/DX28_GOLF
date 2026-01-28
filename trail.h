#pragma once

void InitializeTrail();
void FinalizeTrail();
void UpdateTrail();
void DrawTrail();

void SetTrailPosition(XMFLOAT3 position, int id);
void ResetTrailPosition(XMFLOAT3 position, int id);

// トレイルエフェクトを開始します。IDを返します。
int StartTrailEffect(XMFLOAT3 position);
void FinishTrailEffect(int id);
