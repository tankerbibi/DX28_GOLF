#pragma once

enum class CameraMode
{
	PLAY,
	ROCKET,
	BALL,
	DEBUG,
};

void InitializeCamera();
void FinalizeCamera();
void UpdateCamera();
void DrawCamera();

XMMATRIX GetCameraViewMatrix();
XMMATRIX GetCameraProjectionMatrix();
XMFLOAT3 GetCameraForward();

CameraMode GetCameraMode();