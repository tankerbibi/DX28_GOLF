#include "directx.h"
#include "result.h"
#include "texture.h"
#include "sprite.h"
#include "Keyboard.h"
#include "main.h"
#include "ranking.h"

//　アスペクト比は16：9

static int g_BGTexture;
static int g_ResultTexture;
static int g_PressKeyTexture;
static float g_FrameCount;
static float g_Alpha;

void InitializeResult()
{
	g_BGTexture = TextureLoad(L"asset\\texture\\CoolBall.png");
	g_PressKeyTexture = TextureLoad(L"asset\\texture\\PressKey.png");
	g_FrameCount = 0;
	InitializeRanking();
}

void FinalizeResult()
{
	FinalizeRanking();
}

void UpdateResult()
{
	g_FrameCount = fmodf(g_FrameCount + 0.01f, 2.0f);  // 0, 1, 2を行き来する。

	g_Alpha = fabs(sinf(g_FrameCount * 3.14f));

	if (Keyboard_IsKeyTrigger(KK_ENTER))
	{
		SetScene(SCENE_TITLE);
	}
	UpdateRanking();
}

void DrawResult()
{
	ID3D11ShaderResourceView* texture = GetTexture(g_BGTexture);
	DirectXGetDeviceContext()->PSSetShaderResources(0, 1, &texture);
	SpriteDraw(screenWidth * 0.5f, screenHeight * 0.5f, screenWidth, screenHeight, 0, 0, 1.0f, 1.0f);
	//SpriteDraw(screenWidth * 0.5f, screenHeight * 2.0f / 5.0f, screenWidth * 0.5f, screenHeight * 2 / 3.0f, 0, 0, 1.0f, 1.0f, 1.0f, 1.0f, 0, g_ResultTexture);
	SetAlpha(g_Alpha);
	 texture = GetTexture(g_PressKeyTexture);
	DirectXGetDeviceContext()->PSSetShaderResources(0, 1, &texture);
	SpriteDraw(screenWidth * 0.5f, screenHeight * 6.0f / 7.0f, screenWidth * 3.0f / 5.0f, screenHeight / 7.0f, 0, 0, 1.0f, 1.0f);
	DrawRanking();
}