#include "directX.h"
#include "map.h"
#include "block.h"
#include "ball.h"
#include "sprite.h"
#include "shader.h"
#include "camera.h"

// レンダーターゲットというのを使う.
// 
// テクスチャ本体
static ID3D11Texture2D* g_MapTexture = NULL;
// 書き込み窓口　初　書き込む貯めにはこれが必要
static ID3D11RenderTargetView* g_MapRTV = NULL;
// 参照窓口 今まではこれもあった
static ID3D11ShaderResourceView* g_MapSRV = NULL;

// デプステクスチャ　深度バッファを保存するための変数。普通に順番に描画すると、奥行が狂うので順番を設定する
static ID3D11Texture2D* g_MapDepthTexture = NULL;
// デプス窓口
static ID3D11DepthStencilView* g_MapDSV = NULL;
// 参照するときは必要らしい。

void InitializeMap()
{
	{
		// テクスチャ設定項目を設定
		D3D11_TEXTURE2D_DESC td = {};
		// テクスチャの大きさは２のべき乗が一番やりやすい。メモリの効率が欲、アクセスの速度もはやい。
		td.Width = 256;
		td.Height = 256;
		td.MipLevels = 1;
		td.ArraySize = 1;
		// rgb各８ビット。最近のだと32ビットもある.	
		td.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		td.SampleDesc.Count = 1;
		td.SampleDesc.Quality = 0;
		td.Usage = D3D11_USAGE_DEFAULT;
		// 書き込み用もほしいし、読み込み用もほしい
		td.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		td.CPUAccessFlags = 0;
		td.MiscFlags = 0;
		DirectXGetDevice()->CreateTexture2D(&td, NULL, &g_MapTexture);

		DirectXGetDevice()->CreateShaderResourceView(g_MapTexture, NULL, &g_MapSRV);

		DirectXGetDevice()->CreateRenderTargetView(g_MapTexture, NULL, &g_MapRTV);
	}

	{
		// デブスバッファ作成
		D3D11_TEXTURE2D_DESC td = {};
		// 同じサイズじゃないとダメ
		td.Width = 256;
		td.Height = 256;
		td.MipLevels = 1;
		td.ArraySize = 1;
		
		td.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		td.SampleDesc.Count = 1;
		td.SampleDesc.Quality = 0;
		td.Usage = D3D11_USAGE_DEFAULT;
		td.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		td.CPUAccessFlags = 0;
		td.MiscFlags = 0;
		DirectXGetDevice()->CreateTexture2D(&td, NULL, &g_MapDepthTexture);

		DirectXGetDevice()->CreateDepthStencilView(g_MapDepthTexture, NULL, &g_MapDSV);
	
	}
}

void FinalizeMap()
{
	SAFE_RELEASE(g_MapTexture);
	SAFE_RELEASE(g_MapRTV);
	SAFE_RELEASE(g_MapSRV);

	SAFE_RELEASE(g_MapDepthTexture);
	SAFE_RELEASE(g_MapDSV);

}

void UpdateMap()
{
}

void DrawMap()
{
	// レンダーターゲット設定 レンダーテクスチャに矢印をつないでる
	DirectXGetDeviceContext()->OMSetRenderTargets(1, &g_MapRTV, g_MapDSV);
	// ビューポート設定
	D3D11_VIEWPORT vp;
	vp.Width = 256;
	vp.Height = 256;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	// ずれないように。
	DirectXGetDeviceContext()->RSSetViewports(1, &vp);
	//クリア
	float clearColor[4] = { 0.8f, 0.2f, 0.2f, 0.3f };
	DirectXGetDeviceContext()->ClearRenderTargetView(g_MapRTV, clearColor);
	DirectXGetDeviceContext()->ClearDepthStencilView(g_MapDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);

	// 描画
	// 関数実行
	DrawCameraMap();
	DrawBall();
	Shader_SetPipelineInstance(true);
	DrawBlock();

	// ビューポート設定で初期化
	vp.Width = screenWidth;
	vp.Height = screenHeight;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	DirectXGetDeviceContext()->RSSetViewports(1, &vp);

	Shader_SetPipelineInstance(false);
	// レンダーターゲットをバックバッファに戻してあげる。
	SetRenderTarget();

	// スプライト描画
	DirectXGetDeviceContext()->PSSetShaderResources(0, 1, &g_MapSRV);
	SpriteDraw(screenWidth - 128.0f, 128.0f, 256.0f, 256.0f, 0.0f, 0.0f, 1.0f, 1.0f);

}
// ビューポートを上手く使うと、画面分割ができる。
