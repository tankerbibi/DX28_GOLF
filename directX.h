#ifndef DIRECTX_H // インクルードガード
#define DIRECTX_H

#pragma once

#define NOMINMAX
#include <DirectXMath.h> //エフェクトとかマトリックスを使うための関数
#include <Windows.h>     // min max関数定義を止めるNOMINMAX必要。
#include <d3d11.h> //DirectX 11の関数のプロトタイプ宣言ヘッダー DirectX11と12では設計思想が違う。12は難しく、GPUを直接操作する。
using namespace DirectX;

struct Vertex // 頂点データ　GPUメモリに保存する。
{
  XMFLOAT3 position; // 頂点座標
  XMFLOAT2 texcoord; // テクスチャ座標
  XMFLOAT3 normal;   // 法線座標

  UINT bone_indices[4];
  float bone_weights[4]; // 各ボーンからの影響度
};

struct Transform {
  XMFLOAT3 position;
  XMFLOAT3 scale;
  XMFLOAT3 rotaiton;
};

// 各インスタンスが持つデータ
struct InstanceData {
  XMMATRIX worldMatrix; // 各ブロックのワールド行列
};

#define SAFE_RELEASE(o)                                                        \
  if (o) {                                                                     \
    (o)->Release();                                                            \
    o = NULL;                                                                  \
  }

static constexpr float screenWidth = 1920.0f;
static constexpr float screenHeight = 1080.0f;

static constexpr float deltaTime = 1.0f / 60.0f;

void DirectXInitialize(
    HWND hWnd); // HWNDとはウインドウの識別子。ウィンドウハンドル
                // hwndがないと呼び出せない。
void DirectXFinalize(void);

ID3D11Device *DirectXGetDevice(void);
void SetRenderTarget();
ID3D11DeviceContext *DirectXGetDeviceContext(void);

void SetFullScreen(bool fullScreen); // 使っていない。

void Clear(void);
void Present(void);

void SetDepthEnable(bool depthEnable);

#endif
