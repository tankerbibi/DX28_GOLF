#include "directX.h"
#include "game.h"
#include "score.h" 
#include "sound.h"
#include "Keyboard.h"
#include "cube.h"
#include "camera.h"
#include "mouse.h"
#include "ball.h"

static int g_BGM{};
static bool g_Pause{false};


void InitializeGame()
{
	g_Pause = false;
	InitializeCamera();
	InitializeScore();
	InitializeCube();
	InitializeMouse();
	InitializeBall();

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
	FinalizeCube();
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
		UpdateCube();
		UpdateMouse();
		UpdateBall();
	}
}

void DrawGame()
{
	SetDepthEnable(true);
	DrawCamera();  // カメラは一番最初に描画関連のデータを更新しなければならない。
	DrawCube();
	DrawBall();

	SetDepthEnable(false);
	DrawScore();
	DrawMouse();
	
}