#include "directx.h"
#include "grass.h"
#include "shader.h"
#include "texture.h"
#include "camera.h"
#include <cmath> // atan2を使うために必要

struct Grass
{
	XMFLOAT3 position;
	bool use;
};

static constexpr int maxGrass = 1000;

static ID3D11Buffer* g_VertexBuffer;
static ID3D11Buffer* g_InstanceBuffer;

// テクスチャID
static int g_Texture = -1;


static Grass g_Grass[maxGrass];

void InitializeGrass()
{
	{
		// 頂点バッファの作成
		D3D11_BUFFER_DESC bd{};
		bd.Usage = D3D11_USAGE_DYNAMIC;
		// 4頂点分
		bd.ByteWidth = sizeof(Vertex) * 4;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		DirectXGetDevice()->CreateBuffer(&bd, nullptr, &g_VertexBuffer);
	}

	{
		D3D11_BUFFER_DESC desc = {};
		// 十字作るために二つ分確保・最大個数分のサイズを確保
		desc.ByteWidth = sizeof(InstanceData) * maxGrass;
		// 毎フレーム更新するため動的に設定
		desc.Usage = D3D11_USAGE_DYNAMIC;
		// 頂点バッファとして扱う
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		// CPUから書き込み可能にする
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		DirectXGetDevice()->CreateBuffer(&desc, nullptr, &g_InstanceBuffer);
	}

	///////////////////頂点バッファ設定開始///////////////////////
	{
		D3D11_MAPPED_SUBRESOURCE msr;
		DirectXGetDeviceContext()->Map(g_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr); //g_VertexBufferのありかを探す。

		Vertex* v = (Vertex*)msr.pData;

		// 3dにおいては、ピクセル単位ではなくメートル単位になる。
		// xが横方向、yが縦方向、zが奥行方向
		//四つの頂点を作る。頭の中でイメージ

		float tx = 0.3f;
		float ty = 0.3f;
		float tw = 0.4f;
		float th = 0.2f;

		v[0].position = { -0.5f, 0.5f, 0.0f };
		v[1].position = { 0.5f, 0.5f, 0.0f };
		v[2].position = { -0.5f, -0.5f, 0.0f };
		v[3].position = { 0.5f, -0.5f, 0.0f };

		v[0].texcoord = { tx,		ty };
		v[1].texcoord = { tx + tw,	ty };
		v[2].texcoord = { tx,		ty + th };
		v[3].texcoord = { tx + tw,	ty + th };

		v[0].normal = { 0.0f, 0.0f, -1.0f };
		v[1].normal = { 0.0f, 0.0f, -1.0f };
		v[2].normal = { 0.0f, 0.0f, -1.0f };
		v[3].normal = { 0.0f, 0.0f, -1.0f };

		DirectXGetDeviceContext()->Unmap(g_VertexBuffer, 0);
	}
	//////////////頂点バッファ設定終了////////////////////

	for (int i = 0; i < maxGrass; i++)
	{
		g_Grass[i].position = { 0.0f,0.0f,0.0f};
		g_Grass[i].use = false;

	}

	// テクスチャ読み込み（草の画像を読み込んでください）
	g_Texture = TextureLoad(L"asset\\texture\\Grassdw.png");
}

void FinalizeGrass()
{
	SAFE_RELEASE(g_VertexBuffer);
	SAFE_RELEASE(g_InstanceBuffer);
}

void UpdateGrass()
{
	// 特に何もしない
}

// 座標を指定して描画する関数に変更
void DrawGrass()
{
	ID3D11DeviceContext* context = DirectXGetDeviceContext();

	// --- バッファをロックしてデータを書き込む ---
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	context->Map(g_InstanceBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

	InstanceData* data = (InstanceData*)mappedResource.pData;
	int drawCount = 0;

	for(int i = 0; i < maxGrass; i++)
	{
		if (g_Grass[i].use == false) continue;

		const XMFLOAT3 cameraPos = GetCameraPosition();

		const XMVECTOR gPos = XMLoadFloat3(&g_Grass[i].position);
		const XMVECTOR cPos = XMLoadFloat3(&cameraPos);

		XMVECTOR forward = XMVectorSubtract(cPos, gPos);

		forward = XMVectorSetY(forward, 0);
		forward = XMVector3Normalize(forward);

		XMVECTOR upVec = XMVectorSet(0, 1, 0, 0);

		XMVECTOR right = XMVector3Cross(upVec, forward);

		// 5. 回転行列を作成 (各ベクトルを軸としてセット)
		XMMATRIX rot = XMMatrixIdentity();
		rot.r[0] = right;   // X軸
		rot.r[1] = upVec;   // Y軸
		rot.r[2] = forward; // Z軸

		// XMMatrixLookAtLHはカメラの視点（ビュー行列）を作るための関数みたい。

		// 6. ワールド行列の合成 (スケーリング * 回転 * 移動)
		XMMATRIX matrixWorld = XMMatrixScaling(2.0f, 2.0f, 2.0f);
		matrixWorld *= rot;
		matrixWorld *= XMMatrixRotationY(XM_PI);
		matrixWorld *= XMMatrixTranslationFromVector(gPos);

		//matrixWorldPair *= XMMatrixRotationY(XM_PIDIV2);

		// バッファに書き込み
		data[drawCount].worldMatrix = matrixWorld;
		drawCount += 1;
	}
	context->Unmap(g_InstanceBuffer, 0);

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

		ID3D11ShaderResourceView* texture = GetTexture(g_Texture);
		DirectXGetDeviceContext()->PSSetShaderResources(0, 1, &texture);

		// 頂点バッファ設定
		UINT stride = sizeof(Vertex);
		UINT offset = 0;
		context->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset); //気を付けて　GetじゃなくてSet

		UINT instanceStride = sizeof(InstanceData);
		UINT instanceOffset = 0;
		context->IASetVertexBuffers(1, 1, &g_InstanceBuffer, &instanceStride, &instanceOffset);
		
		// プリミティブトポロジ設定
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);  // トライアングルストリップ（連続） つまりZの書き方

		context->DrawInstanced(4, drawCount, 0, 0);

		// スロット１をnullptrで上書きして、インスタンスバッファの設定を解除する
		ID3D11Buffer* nullBuffer = nullptr;
		UINT zeroStride = 0;
		UINT zeroOffset = 0;
		context->IASetVertexBuffers(1, 1, &nullBuffer, &zeroStride, &zeroOffset);

		context->IASetVertexBuffers(0, 1, &nullBuffer, &zeroStride, &zeroOffset);  // スロット１の初期化はあってもなくても変わらない。
	}	
}


void CreateGrass(XMFLOAT3 position)
{
	for (int i = 0; i < maxGrass; i++)
	{
		if (g_Grass[i].use == true) continue;
		g_Grass[i].position = position;
		g_Grass[i].use = true;
		break;
	}
}
