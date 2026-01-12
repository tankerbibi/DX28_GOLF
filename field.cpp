#include "directX.h"
#include "field.h"
#include "Keyboard.h"
#include "model.h"
#include "camera.h"
#include "shader.h"
#include "breakableBlock.h"
#include <fstream>  // ファイル読み込み用
#include <string>
#include <sstream>

static constexpr unsigned int typeMax = 5;

// 固定配列を維持
static BLOCK g_Block[blockMax]{};

static MODEL* g_Model[typeMax]{};
static std::string g_ModelName[typeMax];

// 実際にロードされたブロックの数を保持する変数
static unsigned int g_BlockCount = 0;

static ID3D11Buffer* g_InstanceBuffer;



// CSVをロードする関数 (固定配列バージョン)
void LoadFieldData(const char* filename);

BLOCK* GetFieldBlock();


void InitializeField()
{	
	g_ModelName[0] = "block";
	g_ModelName[1] = "tree";
	g_ModelName[2] = "kirby";
	g_Model[0] = ModelLoad("asset\\model\\cube.fbx");
	g_Model[1] = ModelLoad("asset\\model\\tree.fbx");
	g_Model[2] = ModelLoad("asset\\model\\Kirby2.fbx");

	// ここでファイルを読み込む
	LoadFieldData("asset\\data\\level_data.csv");

	// 破壊可能ブロックを登録
	for (int type = 0; type < typeMax; type++)
	{
		for (int index = 0; index < g_BlockCount; index++)
		{
			if (g_Block[index].name == g_ModelName[type])
			{
				if (g_ModelName[type] == "block")  // ground
				{

				}
				else if (g_ModelName[type] == "tree")  // tree
				{
					
				}
				else if (g_ModelName[type] == "kirby")  // カービィ
				{

				}
				else if (g_ModelName[type] == "breakableBlock")
				{
					CreateBreakableBlock(g_Block[index].pos);
				}
			}
		}
	}

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
		ModelRelease(g_Model[i]);
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

	// (0:cube, 1:tree, 2:Kriby)
	for (int type = 0; type < typeMax; type++)
	{
		// バッファをロック
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		context->Map(g_InstanceBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

		InstanceData* data = (InstanceData*)mappedResource.pData;
		// 今回描画する個数
		int drawCount = 0;

		for (int index = 0; index < g_BlockCount; index++)
		{
			if (g_Block[index].name == g_ModelName[type])
			{
				XMMATRIX world = XMMatrixIdentity();

				if (g_ModelName[type] == "block")  // ground
				{
					
				}
				else if (g_ModelName[type] == "tree")  // tree
				{
					world *= XMMatrixScaling(1.0f, 1.0f, 1.0f);
				}
				else if (g_ModelName[type] == "kirby")  // カービィ
				{

				}

				world *= XMMatrixTranslation(g_Block[index].pos.x, g_Block[index].pos.y, g_Block[index].pos.z);
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

			ModelDrawInstanced(g_Model[type], g_InstanceBuffer, drawCount);
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
			// 固定配列にデータを格納
			g_Block[g_BlockCount].name = seglist[0];     // name
			g_Block[g_BlockCount].pos.x = std::stof(seglist[1]);    // X
			g_Block[g_BlockCount].pos.y = std::stof(seglist[2]);    // Y
			g_Block[g_BlockCount].pos.z = std::stof(seglist[3]);    // Z

			g_BlockCount++; // 読み込んだ数をインクリメント
		}
	}

	// DEBUG: 読み込まれなかった残りの配列要素を初期化したい場合はここでループを回す
}