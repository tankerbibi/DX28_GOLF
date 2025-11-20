#pragma once

void InitializeCamera();
void FinalizeCamera();
void UpdateCamera();
void DrawCamera();

XMMATRIX GetCameraViewMatrix();
XMMATRIX GetCameraProjectionMatrix();
XMFLOAT3 GetCameraForward();