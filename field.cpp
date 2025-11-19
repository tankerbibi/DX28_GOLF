#include "directX.h"
#include "field.h"
#include "Keyboard.h"
#include "model.h"
#include "camera.h"
#include "shader.h"


static BLOCK g_Block[blockMax]
{
	{{0.0f, 0.0f, 0.0f}, 0 },

	{{0.0f, 1.0f, 0.0f}, 0 },
	{{1.0f, 1.0f, 0.0f}, 0 },
	{{2.0f, 1.0f, 0.0f}, 0 },
	{{3.0f, 1.0f, 0.0f}, 0 },
	{{4.0f, 1.0f, 0.0f}, 0 },
	{{5.0f, 1.0f, 0.0f}, 0 },
	{{6.0f, 1.0f, 0.0f}, 0 },
	{{7.0f, 1.0f, 0.0f}, 0 },
	{{8.0f, 1.0f, 0.0f}, 0 },
	{{9.0f, 1.0f, 0.0f}, 0 },
	{{10.0f, 1.0f, 0.0f}, 0 },
	{{11.0f, 1.0f, 0.0f}, 0 },
	{{12.0f, 1.0f, 0.0f}, 0 },
	{{0.0f, 1.0f, 1.0f}, 0 },
	{{1.0f, 1.0f, 1.0f}, 0 },
	{{2.0f, 1.0f, 1.0f}, 0 },
	{{3.0f, 1.0f, 1.0f}, 0 },
	{{4.0f, 1.0f, 1.0f}, 0 },
	{{5.0f, 1.0f, 1.0f}, 0 },
	{{6.0f, 1.0f, 1.0f}, 0 },
	{{7.0f, 1.0f, 1.0f}, 0 },
	{{8.0f, 1.0f, 1.0f}, 0 },
	{{9.0f, 1.0f, 1.0f}, 0 },
	{{10.0f, 1.0f, 1.0f}, 0 },
	{{11.0f, 1.0f, 1.0f}, 0 },
	{{12.0f, 1.0f, 1.0f}, 0 },
	{{0.0f, 1.0f, 2.0f}, 0 },
	{{1.0f, 1.0f, 2.0f}, 0 },
	{{2.0f, 1.0f, 2.0f}, 0 },
	{{3.0f, 1.0f, 2.0f}, 0 },
	{{4.0f, 1.0f, 2.0f}, 0 },
	{{5.0f, 1.0f, 2.0f}, 0 },
	{{6.0f, 1.0f, 2.0f}, 0 },
	{{7.0f, 1.0f, 2.0f}, 0 },
	{{8.0f, 1.0f, 2.0f}, 0 },
	{{9.0f, 1.0f, 2.0f}, 0 },
	{{10.0f, 1.0f, 2.0f}, 0 },
	{{11.0f, 1.0f, 2.0f}, 0 },
	{{12.0f, 1.0f, 2.0f}, 0 },
	{{0.0f, 1.0f, 3.0f}, 0 },
	{{1.0f, 1.0f, 3.0f}, 0 },
	{{2.0f, 1.0f, 3.0f}, 0 },
	{{3.0f, 1.0f, 3.0f}, 0 },
	{{4.0f, 1.0f, 3.0f}, 0 },
	{{5.0f, 1.0f, 3.0f}, 0 },
	{{6.0f, 1.0f, 3.0f}, 0 },
	{{7.0f, 1.0f, 3.0f}, 0 },
	{{8.0f, 1.0f, 3.0f}, 0 },
	{{9.0f, 1.0f, 3.0f}, 0 },
	{{10.0f, 1.0f, 3.0f}, 0 },
	{{11.0f, 1.0f, 3.0f}, 0 },
	{{12.0f, 1.0f, 3.0f}, 0 },

	{{0.0f, 1.0f, 6.0f}, 0 },
	{{1.0f, 1.0f, 6.0f}, 0 },
	{{2.0f, 1.0f, 6.0f}, 0 },
	{{3.0f, 1.0f, 6.0f}, 0 },
	{{4.0f, 1.0f, 6.0f}, 0 },
	{{5.0f, 1.0f, 6.0f}, 0 },
	{{6.0f, 1.0f, 6.0f}, 0 },
	{{7.0f, 1.0f, 6.0f}, 0 },
	{{8.0f, 1.0f, 6.0f}, 0 },
	{{9.0f, 1.0f, 6.0f}, 0 },
	{{10.0f, 1.0f, 6.0f}, 0 },
	{{11.0f, 1.0f, 6.0f}, 0 },
	{{12.0f, 1.0f, 6.0f}, 0 },
	{{0.0f, 1.0f, 7.0f}, 0 },
	{{1.0f, 1.0f, 7.0f}, 0 },
	{{2.0f, 1.0f, 7.0f}, 0 },
	{{3.0f, 1.0f, 7.0f}, 0 },
	{{4.0f, 1.0f, 7.0f}, 0 },
	{{5.0f, 1.0f, 7.0f}, 0 },

	{{5.0f, 5.0f, 7.0f}, 0 },
	{{6.0f, 5.0f, 7.0f}, 0 },
	{{6.0f, 2.0f, 7.0f}, 0 },
	{{7.0f, 5.0f, 7.0f}, 0 },

	{{6.0f, 1.0f, 7.0f}, 0 },
	{{7.0f, 1.0f, 7.0f}, 0 },
	{{8.0f, 1.0f, 7.0f}, 0 },
	{{9.0f, 1.0f, 7.0f}, 0 },
	{{10.0f, 1.0f, 7.0f}, 0 },
	{{11.0f, 1.0f, 7.0f}, 0 },
	{{12.0f, 1.0f, 7.0f}, 0 },
	{{0.0f, 1.0f, 8.0f}, 0 },
	{{1.0f, 1.0f, 8.0f}, 0 },
	{{2.0f, 1.0f, 8.0f}, 0 },
	{{3.0f, 1.0f, 8.0f}, 0 },
	{{4.0f, 1.0f, 8.0f}, 0 },
	{{5.0f, 1.0f, 8.0f}, 0 },
	{{6.0f, 1.0f, 8.0f}, 0 },
	{{7.0f, 1.0f, 8.0f}, 0 },
	{{8.0f, 1.0f, 8.0f}, 0 },
	{{9.0f, 1.0f, 8.0f}, 0 },
	{{10.0f, 1.0f, 8.0f}, 0 },
	{{11.0f, 1.0f, 8.0f}, 0 },
	{{12.0f, 1.0f, 8.0f}, 0 },
	{{0.0f, 1.0f, 9.0f}, 0 },
	{{1.0f, 1.0f, 9.0f}, 0 },
	{{2.0f, 1.0f, 9.0f}, 0 },
	{{3.0f, 1.0f, 9.0f}, 0 },
	{{4.0f, 1.0f, 9.0f}, 0 },
	{{5.0f, 1.0f, 9.0f}, 0 },
	{{6.0f, 1.0f, 9.0f}, 0 },
	{{7.0f, 1.0f, 9.0f}, 0 },
	{{8.0f, 1.0f, 9.0f}, 0 },
	{{9.0f, 1.0f, 9.0f}, 0 },
	{{10.0f, 1.0f, 9.0f}, 0 },
	{{11.0f, 1.0f, 9.0f}, 0 },
	{{12.0f, 1.0f, 9.0f}, 0 },


	{ {0.0f, 1.5f, 4.0f}, 1 },
	{ {1.0f, 1.5f, 4.0f}, 1 },
	{ {2.0f, 1.5f, 4.0f}, 1 },
	{ {3.0f, 1.5f, 4.0f}, 1 },
	{ {4.0f, 1.5f, 4.0f}, 1 },
	{ {5.0f, 1.5f, 4.0f}, 1 },
	{ {6.0f, 1.5f, 4.0f}, 1 },
	{ {7.0f, 1.5f, 4.0f}, 1 },
	{ {8.0f, 1.5f, 4.0f}, 1 },
	{ {9.0f, 1.5f, 4.0f}, 1 },
	{ {10.0f, 1.5f, 4.0f}, 1 },
	{ {11.0f, 1.5f, 4.0f}, 1 },
	{ {12.0f, 1.5f, 4.0f}, 1 },

	{ {0.0f, 1.5f, 5.0f}, 1 },
	{ {1.0f, 1.5f, 5.0f}, 1 },
	{ {2.0f, 1.5f, 5.0f}, 1 },
	{ {3.0f, 1.5f, 5.0f}, 1 },
	{ {4.0f, 1.5f, 5.0f}, 1 },
	{ {5.0f, 1.5f, 5.0f}, 1 },
	{ {6.0f, 1.5f, 5.0f}, 1 },
	{ {7.0f, 1.5f, 5.0f}, 1 },
	{ {8.0f, 1.5f, 5.0f}, 1 },
	{ {9.0f, 1.5f, 5.0f}, 1 },
	{ {10.0f, 1.5f, 5.0f}, 1 },
	{ {11.0f, 1.5f, 5.0f}, 1 },
	{ {12.0f, 1.5f, 5.0f}, 1 },
	{ {12.0f, 1.5f, 5.0f}, 1 },

	{ {12.0f, 1.5f, 12.0f}, 2 },
};

static XMFLOAT3 g_Rotation;  // pi

static MODEL* g_Model[3];


BLOCK* GetFieldBlock()
{
	return g_Block;
}

void InitializeField()
{
	g_Model[0] = ModelLoad("asset\\model\\cube.fbx");
	g_Model[1] = ModelLoad("asset\\model\\tree.fbx");
	g_Model[2] = ModelLoad("asset\\model\\Kirby2.fbx");
	g_Rotation = {0.0f, 0.0f, 0.0f};
	// g_FieldPos = XMFLOAT3(0.0f, 0.0f, 0.0f);  何が違う？
}

void FinalizeField()
{
	for (int i = 0; i < 3; i++)
	{
		ModelRelease(g_Model[i]);
	}
}

void UpdateField()
{
	//g_Rotation.y += 0.01f;
}

void DrawField()
{
	Shader_Begin();  // シェーダーの設定
	// 頂点シェーダーに変換行列を設定

	for (int i = 0; i < blockMax; i++)
	{
		MATRIX matrix;

		matrix.matrix = XMMatrixIdentity();  // 行列を作成　float 4 x 4
		matrix.matrixWorld = XMMatrixIdentity();  // 行列を作成　float 4 x 4

		if (i == 0)  // 広い床の時
		{
			matrix.matrixWorld *= XMMatrixScaling(100.0f, 1.0f, 100.0f);  // 拡大縮小マトリクス
			matrix.matrixWorld *= XMMatrixRotationRollPitchYaw(XM_PI, g_Rotation.y, g_Rotation.z);  // 回転マトリクス
			matrix.matrixWorld *= XMMatrixTranslation(g_Block[i].pos.x, g_Block[i].pos.y, g_Block[i].pos.z);  // 移動マトリクス。gpuで計算されている。
		}
		else
		{
			if (g_Block[i].type == 2)  // カービーの時
			{
				matrix.matrixWorld *= XMMatrixScaling(0.6f, 0.6f, 0.6f);
				matrix.matrixWorld *= XMMatrixRotationRollPitchYaw(-XM_PIDIV2, 0.0f, 0.0f);
				matrix.matrixWorld *= XMMatrixTranslation(g_Block[i].pos.x, g_Block[i].pos.y, g_Block[i].pos.z);
			}
			else
			{
				matrix.matrixWorld *= XMMatrixScaling(1.0f, 1.0f, 1.0f);
				matrix.matrixWorld *= XMMatrixRotationRollPitchYaw(0.0f, 0.0f, 0.0f);
				matrix.matrixWorld *= XMMatrixTranslation(g_Block[i].pos.x, g_Block[i].pos.y, g_Block[i].pos.z);
			}
			
		}

		matrix.matrix = matrix.matrixWorld;

		matrix.matrix *= GetCameraViewMatrix();  // ビューマトリクス
		matrix.matrix *= GetCameraProjectionMatrix();  // プロジェクションマトリクス

		Shader_SetMatrix(matrix);
		ModelDraw(g_Model[g_Block[i].type]);
	}
}
