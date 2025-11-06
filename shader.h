/*==============================================================================

   シェーダー [shader.h]
														 Author : Youhei Sato
														 Date   : 2025/05/15
--------------------------------------------------------------------------------
プロジェクトファイルでテキストドキュメントを新規作成。名前をshaderPixel2D.hlsl shaderVertex2D.hlslにした。
==============================================================================*/
#ifndef SHADER_H
#define	SHADER_H

#include <d3d11.h>
#include <DirectXMath.h>

struct LIGHT
{
	BOOL lightEnable;  // 4バイト
	float dummy0[3];  // 12バイト
	XMFLOAT3 lightDirection;  // float 4バイト　int 4バイト  4*3=12
	float dummy1;
};

bool Shader_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void Shader_Finalize();

void Shader_SetMatrix(const DirectX::XMMATRIX& matrix);
void Shader_SetLight(const LIGHT& light);

void Shader_Begin();

#endif // SHADER_H
