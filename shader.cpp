/*==============================================================================

   シェーダー [shader.cpp]
														 Author : Youhei Sato
														 Date   : 2025/05/15
--------------------------------------------------------------------------------
シェーダー言語とcpu言語の橋渡しの役割
==============================================================================*/
#include <d3d11.h>
#include <DirectXMath.h>
using namespace DirectX;
#include "directx.h"
#include "debug_ostream.h"
#include <fstream>
#include "shader.h"


static ID3D11VertexShader* g_pVertexShader = nullptr;
// インスタンス描画用頂点シェーダー
static ID3D11VertexShader* g_pVertexShaderInstance = nullptr;

static ID3D11InputLayout* g_pInputLayout = nullptr;
// インスタンス用入力レイアウト
static ID3D11InputLayout* g_pInputLayoutInstance = nullptr;

static ID3D11Buffer* g_pVSConstantBuffer = nullptr;
// ライトにまつわる色々Buffer
static ID3D11Buffer* g_pVSLightBuffer = nullptr;
static ID3D11PixelShader* g_pPixelShader = nullptr;
static ID3D11SamplerState* g_SamplerState = nullptr;

// 注意！初期化で外部から設定されるもの。Release不要。
static ID3D11Device* g_pDevice = nullptr;
static ID3D11DeviceContext* g_pContext = nullptr;

bool Shader_LoadShaderVertex3D(HRESULT* hr);
bool Shader_LoadShaderVertex3DInstance(HRESULT* hr);

bool Shader_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	HRESULT hr; // 戻り値格納用

	// デバイスとデバイスコンテキストのチェック
	if (!pDevice || !pContext) {
		hal::dout << "Shader_Initialize() : 与えられたデバイスかコンテキストが不正です" << std::endl;
		return false;
	}

	// デバイスとデバイスコンテキストの保存
	g_pDevice = pDevice;
	g_pContext = pContext;

	if (!Shader_LoadShaderVertex3D(&hr))
	{
		return false;
	}

	if (!Shader_LoadShaderVertex3DInstance(&hr))
	{
		return false;
	}

	// 頂点シェーダー用定数バッファの作成
	D3D11_BUFFER_DESC buffer_desc{};
	buffer_desc.ByteWidth = sizeof(MATRIX); // バッファのサイズ
	buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER; // バインドフラグ
	g_pDevice->CreateBuffer(&buffer_desc, nullptr, &g_pVSConstantBuffer);

	buffer_desc.ByteWidth = sizeof(LIGHT);  // directX11には16バイト区切りでなければいけないルールがある。GPU都合。
	g_pDevice->CreateBuffer(&buffer_desc, nullptr, &g_pVSLightBuffer);

	// 事前コンパイル済みピクセルシェーダーの読み込み
	std::ifstream ifs_ps("shaderPixel3D.cso", std::ios::binary);
	if (!ifs_ps) {
		MessageBox(nullptr, "ピクセルシェーダーの読み込みに失敗しました\n\nshader_pixel_2d.cso", "エラー", MB_OK);
		return false;
	}

	ifs_ps.seekg(0, std::ios::end);
	std::streamsize filesize = ifs_ps.tellg();
	ifs_ps.seekg(0, std::ios::beg);

	unsigned char* psbinary_pointer = new unsigned char[filesize];
	ifs_ps.read((char*)psbinary_pointer, filesize);
	ifs_ps.close();

	// ピクセルシェーダーの作成
	hr = g_pDevice->CreatePixelShader(psbinary_pointer, filesize, nullptr, &g_pPixelShader);

	delete[] psbinary_pointer; // バイナリデータのバッファを解放

	if (FAILED(hr)) {
		hal::dout << "Shader_Initialize() : ピクセルシェーダーの作成に失敗しました" << std::endl;
		return false;
	}

	//サンプラーステート設定 テクスチャの貼り付け方を色々設定するところ
	D3D11_SAMPLER_DESC samplerDesc = {  };
	samplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;  //D3D11_TEXTURE_ADDRESS_WARPは四人表示されたりする。
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0;
	samplerDesc.MaxAnisotropy = 16;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	ID3D11SamplerState* samplerState;
	g_pDevice->CreateSamplerState(&samplerDesc, &g_SamplerState); //設定が詰まってる

	g_pContext->PSSetSamplers(0, 1, &g_SamplerState);
	return true;
}

void Shader_Finalize()
{
	SAFE_RELEASE(g_SamplerState);
	SAFE_RELEASE(g_pPixelShader);
	SAFE_RELEASE(g_pVSConstantBuffer);
	SAFE_RELEASE(g_pVSLightBuffer);
	SAFE_RELEASE(g_pInputLayout);
	SAFE_RELEASE(g_pVertexShader);
	SAFE_RELEASE(g_pVertexShaderInstance);
}

void Shader_SetMatrix(const MATRIX& matrix)  // gpuに、定数バッファを経由してデータを送る
{
	// 定数バッファ格納用行列の構造体を定義
	XMFLOAT4X4 transpose[2];

	// 行列を転置して定数バッファ格納用行列に変換  そのほうが効率が良いらしい。
	XMStoreFloat4x4(&transpose[0], XMMatrixTranspose(matrix.matrix));
	XMStoreFloat4x4(&transpose[1], XMMatrixTranspose(matrix.matrixWorld));

	// 定数バッファに行列をセット
	g_pContext->UpdateSubresource(g_pVSConstantBuffer, 0, nullptr, transpose, 0, 0);
}

void Shader_SetLight(const LIGHT& light )  // gpuに、定数バッファを経由してデータを送る
{
	// 定数バッファにライトをセット
	g_pContext->UpdateSubresource(g_pVSLightBuffer, 0, nullptr, &light, 0, 0);
}

// この関数は、全体で一回だけ呼び出す関数 DirectXはで一回やった設定がずーッと保存される
void Shader_Begin()
{
	// 頂点シェーダーとピクセルシェーダーを描画パイプラインに設定
	g_pContext->VSSetShader(g_pVertexShader, nullptr, 0);
	g_pContext->PSSetShader(g_pPixelShader, nullptr, 0);

	// 頂点レイアウトを描画パイプラインに設定
	g_pContext->IASetInputLayout(g_pInputLayout);

	// 定数バッファを描画パイプラインに設定(ここを経由して描画してほしい)
	g_pContext->VSSetConstantBuffers(0, 1, &g_pVSConstantBuffer);  // 引数の一個目は、どこからメモリを入れるか設定できる。
	g_pContext->VSSetConstantBuffers(1, 1, &g_pVSLightBuffer);
}

void Shader_SetPipeline(bool isInstance)
{
	if (isInstance)
	{
		g_pContext->VSSetShader(g_pVertexShaderInstance, nullptr, 0);
		g_pContext->IASetInputLayout(g_pInputLayoutInstance);  // 入力レイアウト
	}
	else
	{
		g_pContext->VSSetShader(g_pVertexShader, nullptr, 0);
		g_pContext->IASetInputLayout(g_pInputLayout);
	}
	g_pContext->PSSetShader(g_pPixelShader, nullptr, 0);

	// 定数バッファを描画パイプラインに設定(ここを経由して描画してほしい)
	g_pContext->VSSetConstantBuffers(0, 1, &g_pVSConstantBuffer);  // 引数の一個目は、どこからメモリを入れるか設定できる。
	g_pContext->VSSetConstantBuffers(1, 1, &g_pVSLightBuffer);
}

bool Shader_LoadShaderVertex3D(HRESULT* hr)
{
	// 事前コンパイル済み頂点シェーダーの読み込み
	std::ifstream ifs_vs("shaderVertex3D.cso", std::ios::binary);

	if (!ifs_vs) {
		MessageBox(nullptr, "頂点シェーダーの読み込みに失敗しました\n\nshader_vertex_3d.cso", "エラー", MB_OK);
		return false;
	}

	// ファイルサイズを取得
	ifs_vs.seekg(0, std::ios::end); // ファイルポインタを末尾に移動
	std::streamsize filesize = ifs_vs.tellg(); // ファイルポインタの位置を取得（つまりファイルサイズ）
	ifs_vs.seekg(0, std::ios::beg); // ファイルポインタを先頭に戻す

	// バイナリデータを格納するためのバッファを確保
	unsigned char* vsbinary_pointer = new unsigned char[filesize];

	ifs_vs.read((char*)vsbinary_pointer, filesize); // バイナリデータを読み込む
	ifs_vs.close(); // ファイルを閉じる

	// 頂点シェーダーの作成
	*hr = g_pDevice->CreateVertexShader(vsbinary_pointer, filesize, nullptr, &g_pVertexShader);

	if (FAILED(hr)) {
		hal::dout << "Shader_Initialize() : 頂点シェーダーの作成に失敗しました" << std::endl;
		delete[] vsbinary_pointer; // メモリリークしないようにバイナリデータのバッファを解放
		return false;
	}


	// 頂点レイアウトの定義
	D3D11_INPUT_ELEMENT_DESC layout[] = {  // GPUに正しくデータを送るための設定
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,  0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },  // 3要素32bitが入っている。rgbは関係ない。
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,     0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	UINT num_elements = ARRAYSIZE(layout); // 配列の要素数を取得

	// 頂点レイアウトの作成
	*hr = g_pDevice->CreateInputLayout(layout, num_elements, vsbinary_pointer, filesize, &g_pInputLayout);

	delete[] vsbinary_pointer; // バイナリデータのバッファを解放

	if (FAILED(hr)) {
		hal::dout << "Shader_Initialize() : 頂点レイアウトの作成に失敗しました" << std::endl;
		return false;
	}
}

bool Shader_LoadShaderVertex3DInstance(HRESULT* hr)
{
// 事前コンパイル済み頂点シェーダーの読み込み
	std::ifstream ifs_vs("shaderVertex3DInstance.cso", std::ios::binary);

	if (!ifs_vs) {
		MessageBox(nullptr, "頂点シェーダーの読み込みに失敗しました\n\nshader_vertex_3d_instance.cso", "エラー", MB_OK);
		return false;
	}

	// ファイルサイズを取得
	ifs_vs.seekg(0, std::ios::end); // ファイルポインタを末尾に移動
	std::streamsize filesize = ifs_vs.tellg(); // ファイルポインタの位置を取得（つまりファイルサイズ）
	ifs_vs.seekg(0, std::ios::beg); // ファイルポインタを先頭に戻す

	// バイナリデータを格納するためのバッファを確保
	unsigned char* vsbinary_pointer = new unsigned char[filesize];

	ifs_vs.read((char*)vsbinary_pointer, filesize); // バイナリデータを読み込む
	ifs_vs.close(); // ファイルを閉じる

	// 頂点シェーダーの作成
	*hr = g_pDevice->CreateVertexShader(vsbinary_pointer, filesize, nullptr, &g_pVertexShader);

	if (FAILED(hr)) {
		hal::dout << "Shader_Initialize() : 頂点シェーダーの作成に失敗しました" << std::endl;
		delete[] vsbinary_pointer; // メモリリークしないようにバイナリデータのバッファを解放
		return false;
	}


	// 頂点レイアウトの定義
	D3D11_INPUT_ELEMENT_DESC layout[] = {  // GPUに正しくデータを送るための設定
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,  0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },  // 3要素32bitが入っている。rgbは関係ない。
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,     0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },

		{ "INSTANCE", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0,  D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "INSTANCE", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "INSTANCE", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "INSTANCE", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
	};

	UINT num_elements = ARRAYSIZE(layout); // 配列の要素数を取得

	// 頂点レイアウトの作成
	*hr = g_pDevice->CreateInputLayout(layout, num_elements, vsbinary_pointer, filesize, &g_pInputLayoutInstance);

	delete[] vsbinary_pointer; // バイナリデータのバッファを解放

	if (FAILED(hr)) {
		hal::dout << "Shader_Initialize() : 頂点レイアウトの作成に失敗しました" << std::endl;
		return false;
	}
}
