#pragma once
#include <DirectXMath.h>

using namespace DirectX;

enum class CameraMode { PLAY, DEBUG, COUNT };

void InitializeCamera();
void FinalizeCamera();
void UpdateCamera();
void DrawCamera();

void DrawCameraMap();

XMMATRIX GetCameraViewMatrix();
XMMATRIX GetCameraProjectionMatrix();
XMFLOAT3 GetCameraForward();
XMFLOAT3 GetCameraPosition();

CameraMode GetCameraMode();
void SetCameraMode(CameraMode newCameraMode);

void SetCameraShake(float shake);
