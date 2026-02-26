#include "directX.h"
#include "slope.h"
#include "camera.h"
#include "shader.h"
#include "texture.h"
#include "model.h"

static constexpr int maxHealth = 1000;

static ID3D11Buffer* g_InstanceBuffer;  // インスタンス描画用のバッファ

static MODEL* g_Model[4];

static int g_Texture;

static Slope g_Slope[slopeMax];

static int g_SlopeRadius = 1.5f;

void InitializeSlope()
{
	g_Model[0] = ModelLoad("asset\\model\\slope0.fbx");
	g_Model[1] = ModelLoad("asset\\model\\slope1.fbx");
	g_Model[2] = ModelLoad("asset\\model\\slope2.fbx");
	g_Model[3] = ModelLoad("asset\\model\\slope3.fbx");
	for (int i = 0; i < slopeMax; i++)
	{
		g_Slope[i].position = { 0.0f, 0.0f, 0.0f };
		g_Slope[i].slopeDirection = FORWARDUP;
		g_Slope[i].use = false;
	}

	D3D11_BUFFER_DESC desc = {};
	// 4,000個分のサイズ
	desc.ByteWidth = sizeof(InstanceData) * slopeMax;
	// 毎フレーム更新するため動的に設定
	desc.Usage = D3D11_USAGE_DYNAMIC;
	// 頂点バッファとして扱う
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	// CPUから書き込み可能にする
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	DirectXGetDevice()->CreateBuffer(&desc, nullptr, &g_InstanceBuffer);

	g_Slope[0].position = { -20 ,0, 24 };
	g_Slope[0].slopeDirection = FORWARDUP;
	g_Slope[0].use = true;

	g_Slope[1].position = { -20 ,0, 30 };
	g_Slope[1].slopeDirection = LEFTUP;
	g_Slope[1].use = true;

	g_Slope[2].position = { -20 ,0, 36};
	g_Slope[2].slopeDirection = RIGHTUP;
	g_Slope[2].use = true;


	g_Slope[3].position = { -20 ,0, 42 };
	g_Slope[3].slopeDirection = BACKWARDUP;
	g_Slope[3].use = true;


	g_Texture = TextureLoad(L"asset\\texture\\block_breakable.png");
}

void FinalizeSlope()
{
	SAFE_RELEASE(g_InstanceBuffer);

	for (int i = 0; i < 3; i++)
	{
		ModelRelease(g_Model[i]);
	}
}

void UpdateSlope()
{
	// 必要であれば更新処理
}

void DrawSlope()
{
	for (int i = 0; i < slopeMax; i++)
	{
		// 頂点シェーダーに変換行列を設定

		if (g_Slope[i].use == false) continue;

		MATRIX matrix;

		matrix.matrix = XMMatrixIdentity();
		matrix.matrixWorld = XMMatrixIdentity();

		matrix.matrixWorld *= XMMatrixScaling(g_SlopeRadius * 2.0f, g_SlopeRadius * 2.0f, g_SlopeRadius * 2.0f);
		matrix.matrixWorld *= XMMatrixTranslation(g_Slope[i].position.x, g_Slope[i].position.y, g_Slope[i].position.z);

		matrix.matrix = matrix.matrixWorld;

		matrix.matrix *= GetCameraViewMatrix();  // ビューマトリクス
		matrix.matrix *= GetCameraProjectionMatrix();  // プロジェクションマトリクス

		Shader_SetMatrix(matrix);
		MODEL* model = g_Model[2];

		switch (g_Slope[i].slopeDirection)
		{
		case FORWARDUP:
			model = g_Model[2];
			break;
		case BACKWARDUP:
			model = g_Model[0];
			break;
		case RIGHTUP:
			model = g_Model[3];
			break;
		case LEFTUP:
			model = g_Model[1];
			break;
		default:
			break;
		}

		ModelDraw(model);
	}
//	ID3D11DeviceContext* context = DirectXGetDeviceContext();
//
//	// バッファをロック
//	D3D11_MAPPED_SUBRESOURCE mappedResource;
//	context->Map(g_InstanceBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
//
//	InstanceData* data = (InstanceData*)mappedResource.pData;
//	// 今回描画する個数
//	int drawCount = 0;
//
//	for (int i = 0; i < slopeMax; i++)
//{
//		XMMATRIX world = XMMatrixIdentity();
//
//		world *= XMMatrixScaling(3.0f, 3.0f, 3.0f);
//
//		world *= XMMatrixTranslation(g_Slope[i].position.x, g_Slope[i].position.y, g_Slope[i].position.z);
//		data[drawCount].worldMatrix = world;
//		drawCount++;
//	}
//	context->Unmap(g_InstanceBuffer, 0);
//
//	if (drawCount > 0)
//	{
//		MATRIX commonMatrices;
//		// 単位行列に初期化
//		commonMatrices.matrixWorld = XMMatrixIdentity();
//		commonMatrices.matrix = XMMatrixIdentity();
//
//		commonMatrices.matrix = GetCameraViewMatrix() * GetCameraProjectionMatrix();
//		Shader_SetMatrix(commonMatrices);
//
//		ModelDrawInstanced(model[0], g_InstanceBuffer, drawCount);
//	}
}

void CreateSlope(XMFLOAT3 position)
{
	for (int i = 0; i < slopeMax; i++)
	{
		if (g_Slope[i].use == true) continue;
		g_Slope[i].position = position;
		g_Slope[i].slopeDirection = FORWARDUP;
		g_Slope[i].use = true;
		break;
	}
}

Slope* GetSlope()
{
	return g_Slope;
}

void SetSlopeRadius(float radius)
{
	g_SlopeRadius = radius;
}

float GetSlopeRadius()
{
	return g_SlopeRadius;
}
