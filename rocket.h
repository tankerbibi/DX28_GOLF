#pragma once
#include <DirectXMath.h>

using namespace DirectX;

class Rocket {
public:
  enum State { STATE_START, STATE_MOVE, STATE_EXPLODED, STATE_INACTIVE };

  Rocket();
  void Initialize(const XMFLOAT3 &position, const XMFLOAT3 &direction);
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
void InitializeRocket();
void FinalizeRocket();
void UpdateRocket();
void DrawRocket();
void CreateRocket(const DirectX::XMFLOAT3 &position,
                  const DirectX::XMFLOAT3 &direction);

// Deprecated or modified accessors (exposed for camera compatibility)
float GetRocketYaw();
float GetRocketPitch();
DirectX::XMFLOAT3 GetRocketPos();
void SetRocketStartPosition(DirectX::XMFLOAT3 newPosition);

// Deprecated or modified accessors (might need to remove or change depending on
// usage) For now, keeping them compatible if possible, or remove if they don't
// make sense for multiple rockets. Since the user said "remove keyboard
// control", these getters might be used for camera. We might need a way to get
// the "focused" rocket or just remove them if unused. Checking usage later. For
// now, I'll comment them out or remove them to force compile errors where they
// are used, so I can fix the call sites to use specific rocket instances or
// logic. However, to keep it simple and compile, I will remove them and see
// what breaks.
