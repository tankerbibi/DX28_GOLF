#pragma once
#include "scene.h"

class TitleScene : public Scene {
public:
  TitleScene();
  ~TitleScene() override;
  void Initialize() override;
  void Finalize() override;
  void Update() override;
  void Draw() override;

private:
  static constexpr int TEXTURE_COUNT = 2;
  int m_BGTexture{0};
  int m_TitleTexture{0};
  int m_PressKeyTexture{0};
  float m_FrameCount{0.0f};
  float m_Alpha{0.0f};
};

// Backwards-compatible C API
void InitializeTitle();
void FinalizeTitle();
void UpdateTitle();
void DrawTitle();