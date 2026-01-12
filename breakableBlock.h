#pragma once

void InitializeBreakableBlock();
void FinalizeBreakableBlock();
void UpdateBreakableBlock();
void DrawBreakableBlock();

XMFLOAT3 GetBreakableBlockPos();
void CreateBreakableBlock(XMFLOAT3 position);

bool ResolveBreakableBlockCollision(XMFLOAT3 position, float radius);
