#pragma once
#include <string>

struct BLOCK
{
	XMFLOAT3 pos;
	std::string name;
};

static constexpr unsigned int blockMax = 40000;

void InitializeField();
void FinalizeField();
void UpdateField();
void DrawField();

BLOCK* GetFieldBlock();