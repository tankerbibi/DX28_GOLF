#pragma once

void InitializeBreakableBlock();
void FinalizeBreakableBlock();
void UpdateBreakableBlock();
void DrawBreakableBlock();

void CreateBreakableBlock(XMFLOAT3 position);

bool ResolveBreakableBlockCollision(XMFLOAT3 position, float radius);
