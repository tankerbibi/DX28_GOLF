#include "directX.h"
#include "game.h"
#include "score.h" 
#include "sound.h"
#include "Keyboard.h"
#include "cube.h"
#include "camera.h"
#include "mouse.h"

static int g_BGM{};
static bool g_Pause{false};


void InitializeGame()
{
	g_Pause = false;
	InitializeCamera();
	InitializeScore();
	InitializeCube();
	InitializeMouse();

	//BGM読み込み
	g_BGM = LoadSound("asset\\sound\\Banana,_Mango,_Pineapple.wav");

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
	}
}

void DrawGame()
{
	DrawCamera();  // カメラは一番最初に描画関連のデータを更新しなければならない。
	DrawScore();
	DrawCube();
	DrawMouse();
}