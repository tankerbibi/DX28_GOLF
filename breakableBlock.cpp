#include "directX.h"
#include "BreakableBlock.h"
#include "Keyboard.h"
#include "model.h"
#include "camera.h"
#include "shader.h"
#include "field.h"
#include "goal.h"
#include "main.h"
#include "ball.h"
#include "effect.h"

struct BreakableBlock
{
	XMFLOAT3 position;
	int health;
	bool use;
};

static constexpr int maxHealth = 1000;
static constexpr unsigned int maxBreakableBlock = 100;

static MODEL* g_Model = nullptr;

static XMFLOAT3 g_Positions[maxBreakableBlock];

static BreakableBlock g_BreakableBlock[maxBreakableBlock];

void InitializeBreakableBlock()
{
	for (int i = 0; i < maxBreakableBlock; i++)
	{
		g_BreakableBlock[i].position = {0.0f, 0.0f, 0.0f};
		g_BreakableBlock[i].health = maxHealth;
		g_BreakableBlock[i].use = false;
	}
	g_Model = ModelLoad("asset\\model\\rocket.fbx");

	CreateBreakableBlock({ 10.0f, 10.0f, 10.0f });
}

void FinalizeBreakableBlock()
{
	ModelRelease(g_Model);
}

void UpdateBreakableBlock()
{	

}

void DrawBreakableBlock()
{
	for (int i = 0; i < maxBreakableBlock; i++)
	{
		if (g_BreakableBlock[i].use == false) continue;

		// 頂点シェーダーに変換行列を設定

		MATRIX matrix;

		matrix.matrix = XMMatrixIdentity();  // 行列を作成　float 4 x 4
		matrix.matrixWorld = XMMatrixIdentity();  // 行列を作成　float 4 x 4

		matrix.matrixWorld *= XMMatrixScaling(10.0f, 10.0f, 10.0f);  // 拡大縮小マトリクス	
		matrix.matrixWorld *= XMMatrixTranslation(g_BreakableBlock[i].position.x, g_BreakableBlock[i].position.y, g_BreakableBlock[i].position.z);
		matrix.matrix = matrix.matrixWorld;

		matrix.matrix *= GetCameraViewMatrix();  // ビューマトリクス
		matrix.matrix *= GetCameraProjectionMatrix();  // プロジェクションマトリクス

		Shader_SetMatrix(matrix);
		ModelDraw(g_Model);
	}
}

XMFLOAT3 GetBreakableBlockPos()
{
	return XMFLOAT3();
}

void CreateBreakableBlock(XMFLOAT3 position)
{
	for (int i = 0; i < maxBreakableBlock; i++)
	{
		if (g_BreakableBlock[i].use == true) continue;
		g_BreakableBlock[i].position = position;
		g_BreakableBlock[i].health = maxHealth;
		g_BreakableBlock[i].use = true;
		break;
	}
}

// 指定された座標と半径に接触しているブロックを壊す関数
bool ResolveBreakableBlockCollision(XMFLOAT3 position, float radius)
{
	bool hitAny = false;
	for (int i = 0; i < maxBreakableBlock; i++)
	{
		if (!g_BreakableBlock[i].use) continue;

		// ブロックとロケットの距離を計算 (簡易的な球体判定)
		// ブロックのサイズがScaling(10.0f)なので、判定半径を調整する必要があります
		float dx = g_BreakableBlock[i].position.x - position.x;
		float dy = g_BreakableBlock[i].position.y - position.y;
		float dz = g_BreakableBlock[i].position.z - position.z;
		float distanceSq = dx * dx + dy * dy + dz * dz;

		float blockHitRadius = 5.0f; // スケールに合わせた判定サイズ
		float totalRadius = radius + blockHitRadius;

		if (distanceSq < (totalRadius * totalRadius))
		{
			// 接触した場合
			g_BreakableBlock[i].health -= 1000; // ダメージを与える
			if (g_BreakableBlock[i].health <= 0)
			{
				CreateEffect(g_BreakableBlock[i].position);
				g_BreakableBlock[i].use = false; // ブロック消滅
			}
			hitAny = true;
		}
	}
	return hitAny;
}
