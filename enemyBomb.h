#pragma once
#include <DirectXMath.h>
#include "trail.h"

class EnemyBomb {
public:
	enum State { STATE_START, STATE_MOVE, STATE_EXPLODED, STATE_INACTIVE };

	EnemyBomb();
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
	void EnemyBombHitCheck();

	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 velocity;
	DirectX::XMFLOAT3 rotation;

	Trail trail;
	State state;
	int stateCount;
	int lifeCount - 0;

	static constexpr int lifeCountMax = 120;
	static constexpr float ExplosionRadius = 5.0f;
	static constexpr float firstSpeed = 6.0f;
	static constexpr float Radius = 3.0f;
	static constexpr float resistance = 0.0f;
	static constexpr float gravity = 0.0f;
	static constexpr float damage = 7.0f;
};

// Global management functions
void InitializeEnemyBomb();
void FinalizeEnemyBomb();
void UpdateEnemyBomb();
void DrawEnemyBomb();
void CreateEnemyBomb(const DirectX::XMFLOAT3& position,
	const DirectX::XMFLOAT3& direction);

DirectX::XMFLOAT3 GetEnemyBombPos();
void SetEnemyBombStartPosition(DirectX::XMFLOAT3 newPosition);
