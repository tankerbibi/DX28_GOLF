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
#include "block.h"
#include "map.h"

static GameScene g_GameScene; // single instance

GameScene::GameScene() = default;
GameScene::~GameScene() = default;

void GameScene::Initialize()
{
    m_Pause = false;
    InitializeBreakableBlock();
    InitializeBackgroundBlock();
    InitializeBlock();
    InitializeGrass();
    InitializeSlope();
    InitializeBillboardTree();
    InitializeCamera();
    InitializeTrail();
    InitializeScore();
    InitializeStroke();
    InitializeMouse();
    InitializeBall();
    InitializeRocket();
    InitializeGoal();
    InitializeEffect();
    InitializeShadow();
    InitializeStart();
    InitializeField();
    InitializeMap();

    XMVECTOR direction{ 0.3f, -1.0f, 0.5f };
    direction = XMVector3Normalize(direction);
    DirectX::XMStoreFloat3(&m_LightDirection, direction);

    m_BGM = LoadSound("asset\\sound\\On_the_Edge_of_Midnight.wav");
    SetVolume(m_BGM, 0.2f);
}

void GameScene::Finalize()
{
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
    FinalizeBlock();
    FinalizeBackgroundBlock();
    FinalizeBillboardTree();
    FinalizeGrass();
    FinalizeStart();
    FinalizeMap();
}

void GameScene::Update()
{
    if (Keyboard_IsKeyTrigger(KK_P))
    {
        m_Pause = !m_Pause;
    }
    if (!m_Pause)
    {
        UpdateCamera();
        UpdateStroke();
        UpdateField();
        UpdateSlope();
        UpdateGrass();
        UpdateBillboardTree();
        UpdateBlock();
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
        UpdateMap();
    }
}

void GameScene::Draw()
{
    LIGHT light;

    SetDepthEnable(true);
    light.lightEnable = true;
    light.lightDirection = m_LightDirection;
    Shader_SetLight(light);

    DrawCamera();

    Shader_SetPipelineInstance(true);
    DrawField();
    DrawBackgroundBlock();
    DrawBreakableBlock();
    DrawBlock();

    Shader_SetPipelineInstance(false);
    DrawSlope();
    DrawRocket();

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

    SetDepthEnable(false);

    DrawStroke();
    DrawMouse();
    SetDepthEnable(true);

    DrawMap();
}

// wrappers
void InitializeGame() { g_GameScene.Initialize(); }
void FinalizeGame() { g_GameScene.Finalize(); }
void UpdateGame() { g_GameScene.Update(); }
void DrawGame() { g_GameScene.Draw(); }