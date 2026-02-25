#pragma once
#include "scene.h"
#include "directx.h"

class GameScene : public Scene {
public:
	GameScene();
	~GameScene() override;
	void Initialize() override;
	void Finalize() override;
	void Update() override;
	void Draw() override;

private:
	int m_BGM{ 0 };
	bool m_Pause{ false };
	XMFLOAT3 m_LightDirection{ 0.f, 0.f, 0.f };
};

// Backwards-compatible C API
void InitializeGame();
void FinalizeGame();
void UpdateGame();
void DrawGame();