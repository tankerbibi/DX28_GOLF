#include "directX.h"

#include <fstream>  // ファイル読み込み用
#include <string>
#include <sstream>

#include "field.h"
#include "Keyboard.h"
#include "model.h"
#include "camera.h"
#include "shader.h"
#include "breakableBlock.h"
#include "BackgroundBlock.h"
#include "grass.h"
#include "ball.h"
#include "start.h"
#include "goal.h"
#include "billboardTree.h"

static constexpr unsigned int typeMax = 4;

// 固定配列を維持
static BLOCK g_Block[blockMax]{};

struct FieldAssetData
{
	BLOCKTYPE blockType;
	MODEL* model;
};

FieldAssetData g_FieldData[typeMax] = 
{
	{BLOCKTYPE::BLOCK, nullptr},
	{BLOCKTYPE::TREE, nullptr},
	{BLOCKTYPE::KIRBY, nullptr},
	{BLOCKTYPE::BREAKABLEBLOCK, nullptr},
};


// 実際にロードされたブロックの数を保持する変数
static unsigned int g_BlockCount = 0;

static ID3D11Buffer* g_InstanceBuffer;



// CSVをロードする関数 (固定配列バージョン)
void LoadFieldData(const char* filename);

BLOCK* GetFieldBlock();


void InitializeField()
{	
	FieldAssetData g_FieldDataDammy[typeMax] =
	{
		{BLOCKTYPE::BLOCK, ModelLoad("asset\\model\\cube.fbx")},
		{BLOCKTYPE::TREE, ModelLoad("asset\\model\\tree.fbx")},
		{BLOCKTYPE::KIRBY,  ModelLoad("asset\\model\\Kirby2.fbx")},
		{BLOCKTYPE::BREAKABLEBLOCK,  ModelLoad("asset\\model\\rocket.fbx")},
	};

	std::swap_ranges(std::begin(g_FieldData), std::end(g_FieldData), std::begin(g_FieldDataDammy));

	// ここでファイルを読み込む
	LoadFieldData("asset\\data\\levelData_1.csv");

	D3D11_BUFFER_DESC desc = {};
	// 4,000個分のサイズ
	desc.ByteWidth = sizeof(InstanceData) * blockMax;
	// 毎フレーム更新するため動的に設定
	desc.Usage = D3D11_USAGE_DYNAMIC;
	// 頂点バッファとして扱う
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	// CPUから書き込み可能にする
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	DirectXGetDevice()->CreateBuffer(&desc, nullptr, &g_InstanceBuffer);
}

void FinalizeField()
{
	for (int i = 0; i < typeMax; i++)
	{
		ModelRelease(g_FieldData[i].model);
	}
	SAFE_RELEASE(g_InstanceBuffer);
}

void UpdateField()
{
	//g_Rotation.y += 0.01f;
}

void DrawField()
{
	ID3D11DeviceContext* context = DirectXGetDeviceContext();

	for (int type = 0; type < typeMax; type++)
	{
		// バッファをロック
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		context->Map(g_InstanceBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

		InstanceData* data = (InstanceData*)mappedResource.pData;
		// 今回描画する個数
		int drawCount = 0;

		for (int i = 0; i < g_BlockCount; i++)
		{
			if (g_Block[i].blockType == g_FieldData[type].blockType)
			{
				XMMATRIX world = XMMatrixIdentity();

				if (g_FieldData[type].blockType == BLOCKTYPE::BLOCK)  // ground
				{
					world *= XMMatrixScaling(3.0f, 3.0f, 3.0f);
				}
				else if (g_FieldData[type].blockType == BLOCKTYPE::TREE)  // tree
				{
					world *= XMMatrixScaling(1.0f, 1.0f, 1.0f);
				}
				else if (g_FieldData[type].blockType == BLOCKTYPE::KIRBY)  // カービィ
				{

				}

				world *= XMMatrixTranslation(g_Block[i].pos.x, g_Block[i].pos.y, g_Block[i].pos.z);
				data[drawCount].worldMatrix = world;
				drawCount++;
			}
		}
		context->Unmap(g_InstanceBuffer, 0);

		if (drawCount > 0)
		{
			MATRIX commonMatrices;
			// 単位行列に初期化
			commonMatrices.matrixWorld = XMMatrixIdentity();
			commonMatrices.matrix = XMMatrixIdentity();

			commonMatrices.matrix = GetCameraViewMatrix() * GetCameraProjectionMatrix();
			Shader_SetMatrix(commonMatrices);

			ModelDrawInstanced(g_FieldData[type].model, g_InstanceBuffer, drawCount);
		}
	}
}

BLOCK* GetFieldBlock()
{
	return g_Block;
}

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
			BLOCKTYPE value = BLOCKTYPE::BLOCK;

			if (seglist[0] == "Block")
			{
				value = BLOCKTYPE::BLOCK;
				CreateGrass({ std::stof(seglist[1]),std::stof(seglist[2]) + 1.0f,std::stof(seglist[3]) });
			}
			else if (seglist[0] == "Tree")
			{
				value = BLOCKTYPE::TREE;
			}
			else if (seglist[0] == "BillboardTree")
			{
				CreateBillboardTree({ std::stof(seglist[1]),std::stof(seglist[2]),std::stof(seglist[3]) });
				continue;
			}
			else if (seglist[0] == "BreakableBlock")
			{
				value = BLOCKTYPE::BREAKABLEBLOCK;
				CreateBreakableBlock({std::stof(seglist[1]),std::stof(seglist[2]),std::stof(seglist[3])});
				continue;
			}
			// バックグラウンドブロック
			else if (seglist[0] == "BackgroundBlock")
			{
				CreateBackgroundBlock({ std::stof(seglist[1]),std::stof(seglist[2]),std::stof(seglist[3]) });
				continue;
			}
			else if (seglist[0] == "StartFlag")
			{
				SetStartFlagPosition({ std::stof(seglist[1]),std::stof(seglist[2]),std::stof(seglist[3]) });
				continue;
			}
			else if (seglist[0] == "StartPoint")
			{
				SetStartPosition({ std::stof(seglist[1]),std::stof(seglist[2]),std::stof(seglist[3]) });
				continue;
			}
			else if (seglist[0] == "GoalFlag")
			{
				SetGoalFlagPosition({ std::stof(seglist[1]),std::stof(seglist[2]),std::stof(seglist[3]) });
				SetGoalPosition({ std::stof(seglist[1]),std::stof(seglist[2]),std::stof(seglist[3]) });
				continue;
			}
			else if (seglist[0] == "GoalPoint")
			{
				SetGoalPosition({ std::stof(seglist[1]),std::stof(seglist[2]),std::stof(seglist[3]) });
				continue;
			}

			// 固定配列にデータを格納
			g_Block[g_BlockCount].blockType = value;
			g_Block[g_BlockCount].pos.x = std::stof(seglist[1]);    // X
			g_Block[g_BlockCount].pos.y = std::stof(seglist[2]);    // Y
			g_Block[g_BlockCount].pos.z = std::stof(seglist[3]);    // Z

			g_BlockCount++; // 読み込んだ数をインクリメント
		}
	}

	// DEBUG: 読み込まれなかった残りの配列要素を初期化したい場合はここでループを回す

}