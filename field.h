#pragma once
#include <string>

enum class BLOCKTYPE
{
	BLOCK,
	TREE,
	KIRBY,
	BREAKABLEBLOCK,
	COUNT,
};



struct BLOCK
{
	XMFLOAT3 pos;
	BLOCKTYPE blockType;
};

static constexpr unsigned int blockMax = 40000;

void InitializeField();
void FinalizeField();
void UpdateField();
void DrawField();

BLOCK* GetFieldBlock();