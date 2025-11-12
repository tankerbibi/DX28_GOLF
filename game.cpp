#include "directX.h"
#include "game.h"
#include "score.h" 
#include "sound.h"
#include "Keyboard.h"
#include "camera.h"
#include "field.h"
#include "mouse.h"
#include "ball.h"
#include "shader.h"

static int g_BGM{};
static bool g_Pause{false};

static XMFLOAT3 g_LightDirection;

void InitializeGame()
{
	g_Pause = false;
	InitializeCamera();
	InitializeScore();
	InitializeMouse();
	InitializeBall();
	InitializeField();

	g_LightDirection = { 0.0f, -1.0f, 0.0f };
	//BGM読み込み
	g_BGM = LoadSound("asset\\sound\\On_the_Edge_of_Midnight.wav");

	//BGM再生
	PlaySound(g_BGM, -1);  // －１で無限示すこの仕様は分かりづらいらしい。

	//BGM音量設定
	SetVolume(g_BGM, 0.2f);
}

void FinalizeGame()
{
	//BGM停止
	StopSoundAll();
	FinalizeScore();
	FinalizeField();
	FinalizeCamera();
	FinalizeMouse();
	FinalizeBall();
}

void UpdateGame()
{
	if (Keyboard_IsKeyTrigger(KK_P))
	{
		g_Pause = !g_Pause;
	}
	if (g_Pause == false)
	{
		UpdateCamera();
		UpdateScore();
		UpdateField();
		UpdateMouse();
		UpdateBall();
		if (g_LightDirection.y > 1.0f)
		{
			g_LightDirection.y -= 0.01f;
		}
		else if (g_LightDirection.y < -1.0f)
		{
			g_LightDirection.y += 0.01f;
		}
	}
}

void DrawGame()
{
	LIGHT light;
	
	SetDepthEnable(true);
	light.lightEnable = true;
	light.lightDirection = g_LightDirection;
	Shader_SetLight(light);

	DrawCamera();  // カメラは一番最初に描画関連のデータを更新しなければならない。
	DrawField();
	DrawBall();

	SetDepthEnable(false);
	light.lightEnable = false;
	Shader_SetLight(light);	

	DrawScore();
	DrawMouse();
	
}