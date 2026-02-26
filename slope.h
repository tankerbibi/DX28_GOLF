#pragma once


enum SlopeDirection
{
	FORWARDUP,
	BACKWARDUP,
	RIGHTUP,
	LEFTUP
};
struct Slope
{
	XMFLOAT3 position;
	SlopeDirection slopeDirection;
	bool use;
};

static constexpr unsigned int slopeMax = 1000;

void InitializeSlope();
void FinalizeSlope();
void UpdateSlope();
void DrawSlope();

void CreateSlope(XMFLOAT3 position);

Slope* GetSlope();

void SetSlopeRadius(float radius);
float GetSlopeRadius();
