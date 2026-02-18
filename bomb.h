#pragma once
#include <DirectXMath.h>

using namespace DirectX;

class Bomb {
public:
	enum State { STATE_START, STATE_MOVE, STATE_EXPLODED, STATE_INACTIVE };

	Bomb();
	void Initialize(const XMFLOAT3& position, const XMFLOAT3& direction);
	void Update();
	void Draw();

	bool IsActive() const { return state != STATE_INACTIVE; }
	XMFLOAT3 GetPosition() const { return position; }
	XMFLOAT3 GetRotation() const { return rotation; }

private:
	void Move();
	bool IsHit();
	void OnHit();
	void PushBall();
	void BombHitCheck();

	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 velocity;
	DirectX::XMFLOAT3 rotation;

	State state;
	int stateCount;

	static constexpr float Radius = 0.25f;
	static constexpr float ExplosionMaxPower = 20.0f;
	static constexpr float ExplosionRadius = 5.0f;
};

// Global management functions
void InitializeBomb();
void FinalizeBomb();
void UpdateBomb();
void DrawBomb();
void CreateBomb(const DirectX::XMFLOAT3& position,
	const DirectX::XMFLOAT3& direction);

DirectX::XMFLOAT3 GetBombPos();
void SetBombStartPosition(DirectX::XMFLOAT3 newPosition);
