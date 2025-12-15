#include "directx.h"

//スタティックリンクライブラリのリンク	必要なライブラリを入れることができるようになる。
#pragma comment(lib, "d3d11.lib")	//libにd3d11.libをリンクしている。そうすれば、libの関数が使えるようになる。

static ID3D11Device* g_Device = NULL;	//グローバル変数にstaticを入れると、同じ名前の変数が各cppに存在しても、衝突しなくなる。
static ID3D11DeviceContext* g_DeviceContext = NULL;
static IDXGISwapChain* g_SwapChain = NULL;
static ID3D11Texture2D* g_DepthStencilTexture = NULL;
static ID3D11RenderTargetView* g_RenderTargetView = NULL;
static ID3D11DepthStencilView* g_DepthStencilView = NULL;

static ID3D11RasterizerState* g_RasterizerState = NULL;
static ID3D11BlendState* g_BlendState = NULL;
static ID3D11DepthStencilState* g_DepthStencilStateDepthDisable = NULL;
static ID3D11DepthStencilState* g_DepthStencilStateDepthEnable = NULL;  // DirectXは設定ごとにオブジェクトしなければならない。

ID3D11Buffer* g_pInstanceBuffer = nullptr;

void DirectXInitialize(HWND hWnd)
{
	//デバイスス、スワップチェイン（）、コンテキスト生成（メッセージを扱うために必要）
	DXGI_SWAP_CHAIN_DESC sd = {};
	sd.BufferCount = 1;
	sd.BufferDesc.Width = screenWidth;	//デファインで作っておいたほうがよさそう。
	sd.BufferDesc.Height = screenHeight;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60; //目指すリフレッシュレート
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;

	D3D_FEATURE_LEVEL feature_level = D3D_FEATURE_LEVEL_11_0;

	HRESULT hr = D3D11CreateDeviceAndSwapChain(NULL,	//色々作れる万能関数
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		0,
		NULL,
		0,
		D3D11_SDK_VERSION,
		&sd,
		&g_SwapChain,
		&g_Device,
		&feature_level,
		&g_DeviceContext);


	if (FAILED(hr))
	{	//メッセージボックスなどによるエラー表示

		return;
	}

	//レンダーターゲットビュー生成 バックバッファに書き込むようにするための入口
	ID3D11Texture2D* pBackBuffer = NULL;
	g_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
	g_Device->CreateRenderTargetView(pBackBuffer, NULL, &g_RenderTargetView);
	pBackBuffer->Release();

	//デプスステンシル用テクスチャー作成
	D3D11_TEXTURE2D_DESC td = {};
	td.Width = sd.BufferDesc.Width;
	td.Height = sd.BufferDesc.Height;
	td.MipLevels = 1;
	td.ArraySize = 1;
	td.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	td.SampleDesc = sd.SampleDesc;
	td.Usage = D3D11_USAGE_DEFAULT;
	td.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	td.CPUAccessFlags = 0;
	td.MiscFlags = 0;
	g_Device->CreateTexture2D(&td, NULL, &g_DepthStencilTexture);

	//デプスステンシルターゲット作成
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvd = {};
	dsvd.Format = td.Format;
	dsvd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvd.Flags = 0;
	g_Device->CreateDepthStencilView(g_DepthStencilTexture, &dsvd, &g_DepthStencilView);

	//デバイスコンテキスト連打ーターゲットビューとデプスステンシルビューを設定
	g_DeviceContext->OMSetRenderTargets(1, &g_RenderTargetView, g_DepthStencilView);

	//ビューボード設定
	D3D11_VIEWPORT vp;
	vp.Width = (FLOAT)screenWidth;
	vp.Height = (FLOAT)screenHeight;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;
	g_DeviceContext->RSSetViewports(1, &vp);

	//　ラスタライザステートの作成 塗りつぶしの設定
	D3D11_RASTERIZER_DESC rd = {};
	rd.FillMode = D3D11_FILL_SOLID;
	rd.CullMode = D3D11_CULL_BACK;  // バックフェースカリング
	rd.DepthClipEnable = TRUE;
	rd.MultisampleEnable = FALSE;
	g_Device->CreateRasterizerState(&rd, &g_RasterizerState);

	//デバイスコンテキストにラスタライザーステートを設定
	g_DeviceContext->RSSetState(g_RasterizerState);


	//ブレンドステート設定　半透明の合成用の設定
	D3D11_BLEND_DESC bd = {};
	bd.AlphaToCoverageEnable = FALSE;
	bd.AlphaToCoverageEnable = FALSE;
	bd.RenderTarget[0].BlendEnable = TRUE;

	bd.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	bd.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	bd.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	bd.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	bd.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	bd.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;

	bd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	float blend_factor[4] = { 0.0f,0.0f,0.0f,0.0f };
	g_Device->CreateBlendState(&bd, &g_BlendState);

	g_DeviceContext->OMSetBlendState(g_BlendState, blend_factor, 0xffffffff);


	//深度ステンシルステート設定
	D3D11_DEPTH_STENCIL_DESC dsd = {};
	dsd.DepthEnable = FALSE;
	dsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsd.DepthFunc = D3D11_COMPARISON_LESS;
	dsd.StencilEnable = FALSE;
	g_Device->CreateDepthStencilState(&dsd, &g_DepthStencilStateDepthDisable);

	dsd.DepthEnable = TRUE;
	g_Device->CreateDepthStencilState(&dsd, &g_DepthStencilStateDepthEnable);

	g_DeviceContext->OMSetDepthStencilState(g_DepthStencilStateDepthDisable, NULL);
}


void DirectXFinalize(void) //DirectXの変数はいらなくなったら、リリースする。
{
	SAFE_RELEASE(g_DepthStencilStateDepthDisable);//マクロ　存在するかどうか勝手に判断してくれる便利なヤツ。
	SAFE_RELEASE(g_BlendState);
	SAFE_RELEASE(g_RasterizerState);

	SAFE_RELEASE(g_DepthStencilView);
	SAFE_RELEASE(g_DepthStencilTexture);
	SAFE_RELEASE(g_RenderTargetView);
	SAFE_RELEASE(g_SwapChain);
	SAFE_RELEASE(g_DeviceContext);
	SAFE_RELEASE(g_Device);

}

ID3D11Device* DirectXGetDevice(void)
{
	return g_Device;
}

ID3D11DeviceContext* DirectXGetDeviceContext(void)
{
	return g_DeviceContext;
}

void SetFullScreen(bool fullScreen)
{
	g_SwapChain->SetFullscreenState(fullScreen, nullptr);
}

//バックバッファクリア
void Clear(void)
{
	float clear_color[4] = { 0.5f,0.5f,0.7f,1.0f };
	g_DeviceContext->ClearRenderTargetView(g_RenderTargetView, clear_color);
	g_DeviceContext->ClearDepthStencilView(g_DepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

//画面に表示！
void Present(void)
{
	//　バックバッファとフロントバッファの交換
	g_SwapChain->Present(0, 0);
}

void SetDepthEnable(bool depthEnable)
{
	if (depthEnable)
	{
		g_DeviceContext->OMSetDepthStencilState(g_DepthStencilStateDepthEnable, NULL);
	}
	else
	{
		g_DeviceContext->OMSetDepthStencilState(g_DepthStencilStateDepthDisable, NULL);
	}
}

void CreateInstancceBuffer()
{
	D3D11_BUFFER_DESC desc = {};
	desc.ByteWidth = sizeof(InstanceData) * blockMax;  // 4,000個分のサイズ
	desc.Usage = D3D11_USAGE_DYNAMIC;  // 毎フレーム更新するため動的に設定
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;  // 頂点バッファとして扱う
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;  // CPUから書き込み可能にする

	g_Device->CreateBuffer(&desc, nullptr, &g_pInstanceBuffer);
}
