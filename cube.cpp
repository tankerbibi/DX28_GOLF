#include "cube.h"

#include "main.h"
#include "directx.h"
#include "shader.h"
#include "texture.h"
#include "camera.h"

static constexpr unsigned int cubeNum = 100;
static constexpr float halfSize = 1.0f;

struct Vertex //頂点データ　GPUメモリに保存する。
{
	XMFLOAT3 position;
	XMFLOAT2 texcoord;
};

static ID3D11Buffer* g_VertexBuffer;  // 頂点バッファ

static ID3D11Buffer* g_IndexBuffer;  // インデックスバッファ

static int g_Texture;

static XMFLOAT3 g_Rotation;

void InitializeCube()
{
	// 頂点バッファ生成
	{
		D3D11_BUFFER_DESC bd{};
		bd.Usage = D3D11_USAGE_DYNAMIC;
		bd.ByteWidth = sizeof(Vertex) * 24;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		DirectXGetDevice()->CreateBuffer(&bd, nullptr, &g_VertexBuffer); //g_VertexBufferはGPUのメモリなのでアクセスできない。
	}

	// インデックスバッファ生成
	{
		D3D11_BUFFER_DESC bd{};
		bd.Usage = D3D11_USAGE_DYNAMIC;
		bd.ByteWidth = sizeof(unsigned int) * 36;
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		DirectXGetDevice()->CreateBuffer(&bd, nullptr, &g_IndexBuffer); //g_VertexBufferはGPUのメモリなのでアクセスできない。
	}

	g_Rotation = { 0.0f, 0.0f, 0.0f };

	g_Texture = TextureLoad(L"asset\\texture\\Wood.png");

	///////////////////Indexバッファ設定開始
	{
		//インデックスバッファにデータを設定
		D3D11_MAPPED_SUBRESOURCE msr;  // 存在範囲を指定できる
		DirectXGetDeviceContext()->Map(g_IndexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr); //g_VertexBufferのありかを探す。

		unsigned int* index = (unsigned int*)msr.pData;

		// 上面
		index[0] = 0;
		index[1] = 1;
		index[2] = 2;
		index[3] = 1;
		index[4] = 3;
		index[5] = 2;

		// 底面
		index[6] = 4;
		index[7] = 5;
		index[8] = 6;
		index[9] = 5;
		index[10] = 7;
		index[11] = 6;

		// 前面
		index[12] = 8;
		index[13] = 9;
		index[14] = 10;
		index[15] = 9;
		index[16] = 11;
		index[17] = 10;

		// 背面
		index[18] = 12;
		index[19] = 13;
		index[20] = 14;
		index[21] = 13;
		index[22] = 15;
		index[23] = 14;

		// 右面
		index[24] = 16;
		index[25] = 17;
		index[26] = 18;
		index[27] = 17;
		index[28] = 19;
		index[29] = 18;

		// 左面
		index[30] = 20;
		index[31] = 21;
		index[32] = 22;
		index[33] = 21;
		index[34] = 23;
		index[35] = 22;

		DirectXGetDeviceContext()->Unmap(g_IndexBuffer, 0);
	}
	///////////////////Indexバッファ設定終了

	///////////////////頂点バッファ設定開始
	//頂点バッファにデータを設定
	D3D11_MAPPED_SUBRESOURCE msr;
	DirectXGetDeviceContext()->Map(g_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr); //g_VertexBufferのありかを探す。

	Vertex* v = (Vertex*)msr.pData;


	// 3dにおいては、ピクセル単位ではなくメートル単位になる。
	// xが横方向、yが縦方向、zが奥行方向
	//四つの頂点を作る。頭の中でイメージ



	// 上面
	v[0].position = { -halfSize, halfSize, halfSize };
	v[1].position = { halfSize, halfSize, halfSize };  
	v[2].position = { -halfSize, halfSize, -halfSize };
	v[3].position = { halfSize, halfSize, -halfSize };  

	// 底面
	v[4].position = { -halfSize, -halfSize, -halfSize };
	v[5].position = { halfSize, -halfSize, -halfSize };
	v[6].position = { -halfSize, -halfSize, halfSize };
	v[7].position = { halfSize, -halfSize, halfSize };

	// 前面
	v[8].position = { -halfSize, halfSize, -halfSize };
	v[9].position = { halfSize, halfSize, -halfSize };  
	v[10].position = { -halfSize, -halfSize, -halfSize };
	v[11].position = { halfSize, -halfSize, -halfSize };

	// 背面
	v[12].position = { halfSize, halfSize, halfSize };
	v[13].position = { -halfSize, halfSize, halfSize };
	v[14].position = { halfSize, -halfSize, halfSize };
	v[15].position = { -halfSize, -halfSize, halfSize };

	// 右面
	v[16].position = { halfSize, halfSize, -halfSize };
	v[17].position = { halfSize, halfSize, halfSize };
	v[18].position = { halfSize, -halfSize, -halfSize };
	v[19].position = { halfSize, -halfSize, halfSize };

	// 左面
	v[20].position = { -halfSize, halfSize, halfSize };
	v[21].position = { -halfSize, halfSize, -halfSize };
	v[22].position = { -halfSize, -halfSize, halfSize };
	v[23].position = { -halfSize, -halfSize, -halfSize };

	float tx = 0.0f;
	float ty = 0.0f;
	float tw = 1.0f;
	float th = 1.0f;

	// 上面
	v[0].texcoord = { tx,		ty };
	v[1].texcoord = { tx + tw,	ty };
	v[2].texcoord = { tx,		ty + th};
	v[3].texcoord = { tx + tw,	ty + th};

	// 底面
	v[4].texcoord = { tx,		ty };
	v[5].texcoord = { tx + tw,	ty };
	v[6].texcoord = { tx,		ty + th };
	v[7].texcoord = { tx + tw,	ty + th };

	// 前面
	v[8].texcoord = { tx,		ty};
	v[9].texcoord = { tx + tw,	ty};
	v[10].texcoord = { tx,		ty + th};
	v[11].texcoord = { tx + tw,	ty + th};

	// 背面
	v[12].texcoord = { tx,		ty};
	v[13].texcoord = { tx + tw,	ty};
	v[14].texcoord = { tx,		ty + th};
	v[15].texcoord = { tx + tw,	ty + th};

	// 右面
	v[16].texcoord = { tx,		ty };
	v[17].texcoord = { tx + tw,	ty };
	v[18].texcoord = { tx,		ty + th };
	v[19].texcoord = { tx + tw,	ty + th };

	// 左面
	v[20].texcoord = { tx,		ty };
	v[21].texcoord = { tx + tw,	ty };
	v[22].texcoord = { tx,		ty + th };
	v[23].texcoord = { tx + tw,	ty + th };


	DirectXGetDeviceContext()->Unmap(g_VertexBuffer, 0);
	//////////////頂点バッファ設定終了
}

void FinalizeCube()
{
	SAFE_RELEASE(g_VertexBuffer);  // 頂点バッファには必ず解放しなければならないというルールがある。
}

void UpdateCube()
{
	//g_Rotation.y += 0.07f;
	// g_Rotation.x += 0.01f;
}

void DrawCube()
{
	// シェーダーの設定
	Shader_Begin();

	for(int i = 0; i < cubeNum; i++)
	{
		//頂点シェーダーに変換行列を設定
		XMMATRIX matrix{ XMMatrixIdentity() };  // 行列を作成　float 4 x 4

		matrix *= XMMatrixScaling(1.0f, 1.0f, 1.0f); //拡大縮小マトリクス

		matrix *= XMMatrixRotationRollPitchYaw(g_Rotation.x, g_Rotation.y, g_Rotation.z); //回転マトリクス

		matrix *= XMMatrixTranslation(i * 2.0f * halfSize, 0.0f, 0.0f);  // 移動マトリクス。gpuで計算されている。

		matrix *= GetCameraViewMatrix();  // ビューマトリクス

		matrix *= GetCameraProjectionMatrix();  // プロジェクションマトリクス

		Shader_SetMatrix(matrix);

		//頂点バッファ設定
		UINT stride = sizeof(Vertex);
		UINT offset = 0;
		DirectXGetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset); //気を付けて　GetじゃなくてSet

		// indexバッファ設定
		DirectXGetDeviceContext()->IASetIndexBuffer(g_IndexBuffer, DXGI_FORMAT_R32_UINT, 0);  // intは4byteで、32bit 

		//プリミティブトポロジ設定
		// DirectXGetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);  // トライアングルストリップ（連続） 
		DirectXGetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);  // トライアングルリスト（独立）

		ID3D11ShaderResourceView* texture = GetTexture(g_Texture);
		DirectXGetDeviceContext()->PSSetShaderResources(0, 1, &texture);

		//ポリゴン描画
		DirectXGetDeviceContext()->DrawIndexed(36, 0, 0);  //頂点数
	}
}