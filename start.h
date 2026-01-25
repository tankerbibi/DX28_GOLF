#pragma once

void InitializeStart();
void FinalizeStart();
void UpdateStart();
void DrawStart();

// ボールをスポーンさせるポジション
XMFLOAT3 GetStartPosition();
void SetStartPosition(XMFLOAT3 position);
// スタートの旗のポジションを設定する
void SetStartFlagPosition(XMFLOAT3 position);

void SetStartShake(float Shake);
