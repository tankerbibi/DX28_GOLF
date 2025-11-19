#pragma once

struct BLOCK
{
	XMFLOAT3 pos;
	int type;
};

static constexpr unsigned int blockMax = 500;

void InitializeField();
void FinalizeField();
void UpdateField();
void DrawField();

BLOCK* GetFieldBlock();