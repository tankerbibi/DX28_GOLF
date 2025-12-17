#include "directX.h"
#include "field.h"
#include "Keyboard.h"
#include "model.h"
#include "camera.h"
#include "shader.h"
#include <fstream>  // ファイル読み込み用
#include <string>
#include <sstream>



// 固定配列を維持
static BLOCK g_Block[blockMax];

// 実際にロードされたブロックの数を保持する変数
static unsigned int g_BlockCount = 0;

static XMFLOAT3 g_Rotation;
static MODEL* g_Model[10];

// CSVをロードする関数 (固定配列バージョン)
void LoadFieldData(const char* filename)
{
	g_BlockCount = 0; // カウントをリセット

	std::ifstream file(filename);
	if (!file)
	{
		// ファイルが見つからない場合のエラー処理
		return;
	}

	std::string line;
	// ファイルから1行ずつ読み込む
	while (std::getline(file, line) && g_BlockCount < blockMax)
	{
		std::stringstream ss(line);
		std::string segment;
		std::vector<std::string> seglist;

		// カンマ区切りで分解
		while (std::getline(ss, segment, ','))
		{
			seglist.push_back(segment);
		}

		if (seglist.size() >= 4)
		{
			// 固定配列にデータを格納
			g_Block[g_BlockCount].type = std::stoi(seglist[0]);     // Type
			g_Block[g_BlockCount].pos.x = std::stof(seglist[1]);    // X
			g_Block[g_BlockCount].pos.y = std::stof(seglist[2]);    // Y
			g_Block[g_BlockCount].pos.z = std::stof(seglist[3]);    // Z

			g_BlockCount++; // 読み込んだ数をインクリメント
		}
	}

	// DEBUG: 読み込まれなかった残りの配列要素を初期化したい場合はここでループを回す
}

BLOCK* GetFieldBlock()
{
	return g_Block;
}

void InitializeField()
{
	g_Model[1] = ModelLoad("asset\\model\\cube.fbx");
	g_Model[2] = ModelLoad("asset\\model\\tree.fbx");
	g_Model[3] = ModelLoad("asset\\model\\Kirby2.fbx");
	g_Rotation = { 0.0f, 0.0f, 0.0f };

	// ここでファイルを読み込む
	LoadFieldData("asset\\data\\level_data.csv");

	CreateInstanceBuffer(blockMax);
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
	Shader_Begin();

	ID3D11DeviceContext* context = DirectXGetDeviceContext();
	ID3D11Buffer* pInstanceBuffer = GetInstanceBuffer();

	// (0:cube, 1:tree, 2:Kriby)
	for (int modelType = 0; modelType < 3; modelType++)
	{
		// バッファをロック
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		context->Map(pInstanceBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

		InstanceData* data = (InstanceData*)mappedResource.pData;
		// 今回描画する個数
		int drawCount = 0;

		for (int i = 0; i < g_BlockCount; i++)
		{
			if (g_Block[i].type == modelType)
			{
				XMMATRIX world = XMMatrixIdentity();

				if (modelType == 0)  // ground
				{
					
				}
				else if (modelType == 1)  // tree
				{
					world *= XMMatrixScaling(1.0f, 2.0f, 1.0f);
				}
				else if (modelType == 2)  // カービィ
				{

				}

				world *= XMMatrixTranslation(g_Block[i].pos.x, g_Block[i].pos.y, g_Block[i].pos.z);

				data[drawCount].worldMatrix = XMMatrixTranspose(world);
				data[drawCount].worldMatrix = world;


				drawCount++;
			}
		}
		context->Unmap(pInstanceBuffer, 0);

		if (drawCount > 0)
		{
			MATRIX commonMatrices;
			commonMatrices.matrixWorld = XMMatrixIdentity();
			commonMatrices.matrix = XMMatrixIdentity();

			commonMatrices.matrix = GetCameraViewMatrix() * GetCameraProjectionMatrix();
			Shader_SetMatrix(commonMatrices);

			ModelDrawInstanced(g_Model[modelType], drawCount);
		}
	}

	//// blockMax ではなく、実際に読み込んだ g_BlockCount の数だけループを回します。
	//for (int i = 0; i < g_BlockCount; i++)
	//{
	//	MATRIX matrix;
	//	matrix.matrix = XMMatrixIdentity();
	//	matrix.matrixWorld = XMMatrixIdentity();

	//	// 元のコードにあった「i == 0 の時だけ広い床」というロジックを Type 0 で判定する形に置き換える
	//	// ※ UnityでType 0（Floor）としてエクスポートされたブロックが対象
	//	if (g_Block[i].type == 0)
	//	{
	//		matrix.matrixWorld *= XMMatrixScaling(100.0f, 1.0f, 100.0f);
	//		matrix.matrixWorld *= XMMatrixRotationRollPitchYaw(XM_PI, g_Rotation.y, g_Rotation.z);
	//		matrix.matrixWorld *= XMMatrixTranslation(g_Block[i].pos.x, g_Block[i].pos.y, g_Block[i].pos.z);
	//	}
	//	else
	//	{
	//		// カービィ
	//		if (g_Block[i].type == 2)
	//		{
	//			matrix.matrixWorld *= XMMatrixScaling(0.6f, 0.6f, 0.6f);
	//			matrix.matrixWorld *= XMMatrixRotationRollPitchYaw(-XM_PIDIV2, 0.0f, 0.0f);
	//		}
	//		else // 木や通常のブロック (Type 1 など)
	//		{
	//			matrix.matrixWorld *= XMMatrixScaling(1.0f, 1.0f, 1.0f);
	//			matrix.matrixWorld *= XMMatrixRotationRollPitchYaw(0.0f, 0.0f, 0.0f);
	//		}

	//		// 共通の移動処理
	//		matrix.matrixWorld *= XMMatrixTranslation(g_Block[i].pos.x, g_Block[i].pos.y, g_Block[i].pos.z);
	//	}

	//	matrix.matrix = matrix.matrixWorld;
	//	matrix.matrix *= GetCameraViewMatrix();
	//	matrix.matrix *= GetCameraProjectionMatrix();

	//	Shader_SetMatrix(matrix);

	//	int modelIndex = g_Block[i].type;
	//	if (modelIndex >= 0 && modelIndex < 3)
	//	{
	//		ModelDraw(g_Model[modelIndex]);
	//	}
	//}
}