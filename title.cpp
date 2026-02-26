#include "directX.h"
#include "title.h"
#include "Keyboard.h"
#include "main.h"
#include "sprite.h"
#include "texture.h"


static TitleScene g_TitleScene; // single instance to preserve original behavior

TitleScene::TitleScene() = default;
TitleScene::~TitleScene() = default;

void TitleScene::Initialize() {
	m_BGTexture = TextureLoad(L"asset\\texture\\Title.png");
	m_PressKeyTexture = TextureLoad(L"asset\\texture\\PressKey.png");
	m_FrameCount = 0.0f;
}

void TitleScene::Finalize() {}

void TitleScene::Update() {
	m_FrameCount = fmodf(m_FrameCount + 0.01f, 2.0f);
	m_Alpha = fabs(sinf(m_FrameCount * 3.14f));

	if (Keyboard_IsKeyTrigger(KK_SPACE)) {
		SetScene(SCENE_GAME);
	}
}

void TitleScene::Draw() {
	ID3D11ShaderResourceView* texture = GetTexture(m_BGTexture);
	DirectXGetDeviceContext()->PSSetShaderResources(0, 1, &texture);
	SpriteDraw(screenWidth * 0.5f, screenHeight * 0.5f, screenWidth, screenHeight, 0, 0, 1.0f, 1.0f);


	texture = GetTexture(m_PressKeyTexture);
	DirectXGetDeviceContext()->PSSetShaderResources(0, 1, &texture);
	SpriteDraw(screenWidth * 0.5f, screenHeight * 5.0f / 7.0f, screenWidth * 3.0f / 5.0f, screenHeight / 7.0f, 0, 0, 1.0f, 1.0f);
}

void InitializeTitle() { g_TitleScene.Initialize(); }
void FinalizeTitle() { g_TitleScene.Finalize(); }
void UpdateTitle() { g_TitleScene.Update(); }
void DrawTitle() { g_TitleScene.Draw(); }
