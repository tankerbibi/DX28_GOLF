#include "directX.h"
#include "rocket.h"
#include "Keyboard.h"
#include "ball.h"
#include "breakableBlock.h"
#include "camera.h"
#include "effect.h"
#include "field.h"
#include "goal.h"
#include "main.h"
#include "model.h"
#include "ranking.h"
#include "shader.h"
#include "stroke.h"
#include <cmath>
#include <vector>

static MODEL* g_Model = nullptr;
static std::vector<Rocket> g_Rockets;

// --- Rocket Class Implementation ---

Rocket::Rocket()
	: position({ 0.0f, 0.0f, 0.0f }), velocity({ 0.0f, 0.0f, 0.0f }),
	rotation({ 0.0f, 0.0f, 0.0f }), state(STATE_INACTIVE), stateCount(0) {
}

void Rocket::Initialize(const DirectX::XMFLOAT3& startPos,
	const DirectX::XMFLOAT3& direction) {
	position = startPos;

	// Normalize direction and set velocity
	float speed = 0.5f; // Adjust speed as needed
	float len = sqrtf(direction.x * direction.x + direction.y * direction.y +
		direction.z * direction.z);
	if (len > 0.0f) {
		velocity.x = (direction.x / len) * speed;
		velocity.y = (direction.y / len) * speed;
		velocity.z = (direction.z / len) * speed;
	}
	else {
		velocity = { 0.0f, 0.0f, 0.5f }; // Default forward if direction is zero
	}

	// Calculate rotation (Yaw/Pitch) from direction for rendering
	// Simple lookup: Yaw is atan2(x, z)
	rotation.y = atan2f(velocity.x, velocity.z);
	// Pitch is atan2(y, sqrt(x^2 + z^2)) roughly, simplified
	float xzLen = sqrtf(velocity.x * velocity.x + velocity.z * velocity.z);
	rotation.x = -atan2f(velocity.y, xzLen);
	rotation.z = 0.0f;

	state = STATE_START;
	stateCount = 0;
}

void Rocket::Update() {
	if (state == STATE_INACTIVE)
		return;

	switch (state) {
	case STATE_START:
		// Optional start delay or animation
		stateCount++;
		if (stateCount >
			10) // Short delay to ensure it doesn't explode immediately on player?
		{
			state = STATE_MOVE;
			stateCount = 0;
		}
		break;

	case STATE_MOVE:
		Move();
		if (IsHit()) {
			OnHit();
		}
		break;

	case STATE_EXPLODED:
		CreateEffectScale(position,
			{ ExplosionRadius, ExplosionRadius, ExplosionRadius });
		stateCount++;
		// Trigger ball push once during explosion
		if (stateCount == 15) {
			PushBall();
		}
		if (stateCount > 60) // End explosion
		{
			state = STATE_INACTIVE;
		}
		break;
	}
}

void Rocket::Draw() {
	if (state != STATE_MOVE && state != STATE_START)
		return; // Don't draw if exploded (effect handles it) or inactive

	Shader_Begin();

	DirectX::XMMATRIX matrixWorld = DirectX::XMMatrixIdentity();
	matrixWorld *= DirectX::XMMatrixScaling(10.0f, 10.0f, 10.0f);
	matrixWorld *=
		DirectX::XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);
	matrixWorld *=
		DirectX::XMMatrixTranslation(position.x, position.y, position.z);

	MATRIX matrix;
	matrix.matrixWorld = matrixWorld;
	matrix.matrix =
		matrixWorld * GetCameraViewMatrix() * GetCameraProjectionMatrix();

	Shader_SetMatrix(matrix);
	ModelDraw(g_Model);
}

void Rocket::Move() {
	// Simple linear movement
	position.x += velocity.x;
	position.y += velocity.y;
	position.z += velocity.z;
	
}

bool Rocket::IsHit() {
	// Collision with Breakable Blocks
	if (ResolveBreakableBlockCollision(position, Radius)) {
		return true;
	}

	// Collision with Static Blocks
	BLOCK* block = GetFieldBlock();

	float blockRadius = 1.5f; // From original code

	for (int i = 0; i < blockMax; i++) {
		if (block[i].blockType != BLOCKTYPE::BLOCK)
			continue;

		// AABB collision check (simplified from original for brevity but keeping
		// logic)
		if (block[i].pos.y - blockRadius < position.y &&
			position.y < block[i].pos.y + blockRadius &&
			block[i].pos.z - blockRadius < position.z &&
			position.z < block[i].pos.z + blockRadius &&
			block[i].pos.x - blockRadius < position.x &&
			position.x < block[i].pos.x + blockRadius) {
			return true;
		}
	}

	// Floor collision (if needed, usually handled by blocks?)
	if (position.y < -10.0f) // Out of bounds
	{
		return true;
	}

	return false;
}

void Rocket::OnHit() {
	state = STATE_EXPLODED;
	stateCount = 0;
}

void Rocket::PushBall() {
	XMFLOAT3 ballPosition = GetBallPosition();
	XMFLOAT3 force;

	force.x = ballPosition.x - position.x;
	force.y = ballPosition.y - position.y;
	force.z = ballPosition.z - position.z;

	float length =
		sqrtf(force.x * force.x + force.y * force.y + force.z * force.z);

	if (length <= ExplosionRadius) {
		float ratio = length / ExplosionRadius;
		if (ratio > 1.0f)
			ratio = 1.0f;
		float power = ExplosionMaxPower * (1.0f - ratio);

		if (length > 0.01f) // Avoid div by zero
		{
			force.x /= length;
			force.y /= length;
			force.z /= length;
		}

		AddForce({ force.x * power * 3.0f, force.y * power + 5.0f,
				  force.z * power * 3.0f });
	}
}

// --- Global Management Functions ---

void InitializeRocket() {
	g_Model = ModelLoad("asset\\model\\Rocket2.fbx");
	g_Rockets.clear();
}

void FinalizeRocket() {
	ModelRelease(g_Model);
	g_Rockets.clear();
}

void UpdateRocket() {
	// Update all rockets
	for (auto it = g_Rockets.begin(); it != g_Rockets.end();) {
		it->Update();
		if (!it->IsActive()) {
			it = g_Rockets.erase(it);
		}
		else {
			++it;
		}
	}
}

void DrawRocket() {
	for (auto& rocket : g_Rockets) {
		rocket.Draw();
	}
}

void CreateRocket(const DirectX::XMFLOAT3& position,
	const DirectX::XMFLOAT3& direction) {
	Rocket newRocket;
	newRocket.Initialize(position, direction);
	g_Rockets.push_back(newRocket);
}

// Deprecated getters implementation (returning latest rocket data)
float GetRocketYaw() {
	if (!g_Rockets.empty())
		return g_Rockets.back().GetRotation().y;
	return 0.0f;
}

float GetRocketPitch() {
	if (!g_Rockets.empty())
		return g_Rockets.back().GetRotation().x;
	return 0.0f;
}

DirectX::XMFLOAT3 GetRocketPos() {
	if (!g_Rockets.empty())
		return g_Rockets.back().GetPosition();
	return { 0.0f, 0.0f, 0.0f };
}

void SetRocketStartPosition(DirectX::XMFLOAT3 newPosition) {}
