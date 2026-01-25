#include "directx.h"
#include "title.h"
#include "texture.h"
#include "sprite.h"
#include "Keyboard.h"
#include "main.h"
//#include "titleCursor.h"

//　アスペクト比は16：9

static int g_BGTexture;
static int g_TitleTexture;
static int g_PressKeyTexture;
static float g_FrameCount;
static float g_Alpha;

void InitializeTitle()
{
	g_BGTexture = TextureLoad(L"asset\\texture\\GOLFTitle.png");
	// g_TitleTexture = TextureLoad(L"asset\\texture\\Title.png");
	g_PressKeyTexture = TextureLoad(L"asset\\texture\\PressKey.png");
	g_FrameCount = 0;
}

void FinalizeTitle()
{
}

void UpdateTitle()
{
	g_FrameCount = fmodf(g_FrameCount + 0.01f, 2.0f);  // 0, 1, 2を行き来する。

	g_Alpha = fabs(sinf(g_FrameCount * 3.14f));

	if (Keyboard_IsKeyTrigger(KK_ENTER))
	{
		SetScene(SCENE_GAME);
	}
}

void DrawTitle()
{
	SpriteDraw(screenWidth * 0.5f, screenHeight * 0.5f, screenWidth, screenHeight, 0,0,1.0f,1.0f,g_BGTexture);
	// SpriteDraw(screenWidth * 0.5f, screenHeight * 2.0f / 5.0f, screenWidth * 0.5f, screenHeight * 2 / 3.0f, 0,0,1.0f,1.0f,g_TitleTexture);
	SetAlpha(g_Alpha);
	SpriteDraw(screenWidth * 0.5f, screenHeight * 6.0f / 7.0f, screenWidth * 3.0f / 5.0f, screenHeight / 7.0f, 0,0,1.0f,1.0f, g_PressKeyTexture);
}