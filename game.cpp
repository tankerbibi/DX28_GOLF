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
#include "goal.h"
#include "stroke.h"
#include "rocket.h"
#include "effect.h"
#include "trail.h"
#include "shadow.h"
#include "breakableBlock.h"
#include "grass.h"
#include "start.h"
#include "BackgroundBlock.h"
#include "billboardTree.h"
#include "slope.h"

static int g_BGM{};
static bool g_Pause{false};

static XMFLOAT3 g_LightDirection;

void InitializeGame()
{
	g_Pause = false;
	InitializeBreakableBlock();
	InitializeBackgroundBlock();
	InitializeGrass();
	InitializeSlope();
	InitializeBillboardTree();
	InitializeCamera();
	InitializeScore();
	InitializeStroke();
	InitializeMouse();
	InitializeBall();
	InitializeRocket();
	InitializeGoal();
	InitializeEffect();
	InitializeTrail();
	InitializeShadow();
	InitializeStart();
	InitializeGoal();
	InitializeField();

	XMVECTOR direction{ 0.3f, -1.0f, 0.5f };  // SIMDの機能　XYZを一度に計算できる
	direction = XMVector3Normalize(direction);  // 正規化(長さ)
	DirectX::XMStoreFloat3(&g_LightDirection, direction);

	//BGM読み込み
	g_BGM = LoadSound("asset\\sound\\On_the_Edge_of_Midnight.wav");

	//BGM再生
	// PlaySound(g_BGM, -1);  // －１で無限示すこの仕様は分かりづらいらしい。

	//BGM音量設定
	SetVolume(g_BGM, 0.2f);
}

void FinalizeGame()
{
	//BGM停止
	StopSoundAll();
	FinalizeScore();
	FinalizeStroke();
	FinalizeField();
	FinalizeCamera();
	FinalizeMouse();
	FinalizeBall();
	FinalizeSlope();
	FinalizeRocket();
	FinalizeGoal();
	FinalizeEffect();
	FinalizeTrail();
	FinalizeShadow();
	FinalizeBreakableBlock();
	FinalizeBackgroundBlock();
	FinalizeBillboardTree();
	FinalizeGrass();
	FinalizeStart();
	FinalizeGoal();
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
		// UpdateScore();
		UpdateStroke();
		UpdateField();
		UpdateSlope();
		UpdateGrass();
		UpdateBillboardTree();
		UpdateBackgroundBlock();
		UpdateStart();
		UpdateGoal();


		if (GetCameraMode() == CameraMode::DEBUG)
		{
			UpdateMouse();
		}
		UpdateBall();
		UpdateRocket();
		UpdateGoal();
		UpdateEffect();
		UpdateTrail();
		UpdateShadow();
		UpdateBreakableBlock();

		/*if (g_LightDirection.y > 1.0f)
		{
			g_LightDirection.y -= 0.01f;
		}
		else if (g_LightDirection.y < -1.0f)
		{
			g_LightDirection.y += 0.01f;
		}*/
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

	Shader_SetPipelineInstance(true);
	DrawField();
	DrawBackgroundBlock();
	DrawBreakableBlock();

	Shader_SetPipelineInstance(false);
	DrawSlope();
	DrawRocket();

	// ライトをオフにする
	light.lightEnable = false;
	Shader_SetLight(light);	

	Shader_SetPipelineInstance(true);
	DrawGrass();
	DrawBillboardTree();

	Shader_SetPipelineInstance(false);
	DrawShadow();
	DrawStart();
	DrawGoal();

	DrawTrail();
	DrawEffect();
	DrawBall();

	// 2D描画するときの設定
	SetDepthEnable(false);

	// DrawScore();
	DrawStroke();
	DrawMouse();
}
