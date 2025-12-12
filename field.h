#pragma once

struct BLOCK
{
	XMFLOAT3 pos;
	int type;
};

static constexpr unsigned int blockMax = 4000;

void InitializeField();
void FinalizeField();
void UpdateField();
void DrawField();

BLOCK* GetFieldBlock();