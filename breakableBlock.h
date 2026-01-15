#pragma once


struct BreakableBlock
{
	XMFLOAT3 position;
	int health;
	bool use;
};

static constexpr unsigned int maxBreakableBlock = 1000;

void InitializeBreakableBlock();
void FinalizeBreakableBlock();
void UpdateBreakableBlock();
void DrawBreakableBlock();

void CreateBreakableBlock(XMFLOAT3 position);

bool ResolveBreakableBlockCollision(XMFLOAT3 position, float radius);

BreakableBlock* GetBreakableBlock();
