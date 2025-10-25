#pragma once

void InitializeCamera();
void FinalizeCamera();
void UpdateCamera();
void DrawCamera();

XMMATRIX GetCameraViewMatrix();
XMMATRIX GetCameraProjectionMatrix();

void AddCameraPosX(float x);

void AddCameraPosY(float y);

void AddCameraPosZ(float z);

void AddCameraPos(float x, float y, float z);

void SetCameraTargetPos(XMFLOAT3 newTargetPos);