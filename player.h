#pragma once

class Player
{
private:
	XMFLOAT3 position;
	XMFLOAT3 velocity;
	XMFLOAT3 rotation;

	static constexpr float Radius = 5.0f;

	static constexpr float velocityPower = 10.0f;
	static constexpr float posYGap = 5.0f;

	static constexpr unsigned int animationFrame = 30;
	unsigned int animationCount = 0;

	int currentDrawTexture;  // idle, walkRight, walkLeft などの状態を表す整数

public:
	void InitializePlayer();
	void FinalizePlayer();
	void UpdatePlayer();
	void DrawPlayer();
	XMFLOAT3 GetPosition() { return position; }

private:
	void PlayerHitCheck();

};


void InitializePlayer();
void FinalizePlayer();
void UpdatePlayer();
void DrawPlayer();

XMFLOAT3 GetPlayerPosition();
