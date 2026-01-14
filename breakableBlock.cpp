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
static constexpr unsigned int maxBreakableBlock = 1000;

static MODEL* g_Model = nullptr;

// インスタンス描画用のバッファ
static ID3D11Buffer* g_InstanceBuffer;

static BreakableBlock g_BreakableBlock[maxBreakableBlock];

void InitializeBreakableBlock()
{
	for (int i = 0; i < maxBreakableBlock; i++)
	{
		g_BreakableBlock[i].position = { 0.0f, 0.0f, 0.0f };
		g_BreakableBlock[i].health = maxHealth;
		g_BreakableBlock[i].use = false;
	}

	// モデル読み込み
	g_Model = ModelLoad("asset\\model\\tree.fbx"); // 元のコードの通りrocket.fbxを使用

	// --- インスタンスバッファの作成 (Field.cppと同様の処理) ---
	D3D11_BUFFER_DESC desc = {};
	// 最大個数分のサイズを確保
	desc.ByteWidth = sizeof(InstanceData) * maxBreakableBlock;
	// 毎フレーム更新するため動的に設定
	desc.Usage = D3D11_USAGE_DYNAMIC;
	// 頂点バッファとして扱う
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	// CPUから書き込み可能にする
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	DirectXGetDevice()->CreateBuffer(&desc, nullptr, &g_InstanceBuffer);
}

void FinalizeBreakableBlock()
{
	ModelRelease(g_Model);
	// バッファの解放
	SAFE_RELEASE(g_InstanceBuffer);
}

void UpdateBreakableBlock()
{
	// 必要であれば更新処理
}

void DrawBreakableBlock()
{
	ID3D11DeviceContext* context = DirectXGetDeviceContext();

	// --- 1. バッファをロックしてデータを書き込む ---
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	if (FAILED(context->Map(g_InstanceBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
	{
		return;
	}

	InstanceData* data = (InstanceData*)mappedResource.pData;
	int drawCount = 0;

	for (int i = 0; i < maxBreakableBlock; i++)
	{
		if (g_BreakableBlock[i].use == false) continue;

		// 行列計算
		XMMATRIX world = XMMatrixIdentity();
		world *= XMMatrixScaling(10.0f, 10.0f, 10.0f); // 拡大縮小
		world *= XMMatrixTranslation(g_BreakableBlock[i].position.x, g_BreakableBlock[i].position.y, g_BreakableBlock[i].position.z); // 移動

		// バッファに書き込み
		data[drawCount].worldMatrix = world;
		drawCount++;
	}

	// 書き込み終了
	context->Unmap(g_InstanceBuffer, 0);

	// --- 2. 描画実行 ---
	if (drawCount > 0)
	{
		MATRIX commonMatrices;

		// ワールド行列は個別に計算したので、ここではIdentityにしておく
		commonMatrices.matrixWorld = XMMatrixIdentity();
		commonMatrices.matrix = XMMatrixIdentity();

		// ビュー・プロジェクション行列を計算
		commonMatrices.matrix = GetCameraViewMatrix() * GetCameraProjectionMatrix();

		// シェーダーに共通行列（View * Proj）を設定
		Shader_SetMatrix(commonMatrices);

		// インスタンス描画を実行
		// モデル、インスタンスバッファ、描画する個数を渡す
		ModelDrawInstanced(g_Model, g_InstanceBuffer, drawCount);
	}
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
