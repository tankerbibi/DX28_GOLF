#pragma once

enum class CameraMode
{
	PLAY,
	ROCKET,
	BALL,
	LOOKBALL,
	DEBUG,
	COUNT
};

void InitializeCamera();
void FinalizeCamera();
void UpdateCamera();
void DrawCamera();

XMMATRIX GetCameraViewMatrix();
XMMATRIX GetCameraProjectionMatrix();
XMFLOAT3 GetCameraForward();
XMFLOAT3 GetCameraPosition();

void SetCameraMode(CameraMode newCameraMode);
CameraMode GetCameraMode();