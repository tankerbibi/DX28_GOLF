#ifndef DIRECTX_H //インクルードガード
#define DIRECTX_H

#define NOMIMAX //windows.hでファインの直前に書くルール。 No! Min and Maxマクロ定義。
#include <Windows.h>
#include <d3d11.h> //DirectX 11の関数のプロトタイプ宣言が入ったヘッダー DirectX11と12では設計思想が違う。12は難しすぎるらしい、GPUを直接操作する。
#include <DirectXMath.h> //エフェクトとかマトリックスを使うための関数
using namespace DirectX;

#define SAFE_RELEASE(o)		if (o) { (o)->Release(); o = NULL;}

static constexpr float screenWidth = 1920.0f;
static constexpr float screenHeight = 1080.0f;

static constexpr float pieceWidth = 80.0f;
static constexpr float pieceHeight = 80.0f;

static constexpr int blockCols = 6;
static constexpr int blockRows = 13;


void DirectXInitialize(HWND hWnd); //HWNDとはウインドウの識別子。ウィンドウハンドル hwndがないと呼び出せない。
void DirectXFinalize(void);

ID3D11Device* DirectXGetDevice(void);
ID3D11DeviceContext* DirectXGetDeviceContext(void);

void SetFullScreen(bool fullScreen);  // 使っていない。

void Clear(void);
void Present(void);

void SetDepthEnable(bool depthEnable);

#endif