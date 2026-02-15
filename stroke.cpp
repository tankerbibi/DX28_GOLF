#include "directX.h"
#include "stroke.h"

#include "texture.h"
#include "sprite.h"

//　アスペクト比は16：9

static int g_Texture;
static int g_Stroke;

void InitializeStroke()
{
	g_Texture = TextureLoad(L"asset\\texture\\number_1.png");

}

void FinalizeStroke()
{

}

void UpdateStroke()
{

}

void DrawStroke()
{
	int value = g_Stroke;

	ID3D11ShaderResourceView* texture = GetTexture(g_Texture);
	DirectXGetDeviceContext()->PSSetShaderResources(0, 1, &texture);

	for (int i = 0; i < 3; i++)
	{
		int num = value % 10;
		value /= 10;

		float tx{ (num % 5) / 5.0f };
		float tw{ 1.0f / 5.0f };
		float ty{ (num / 5) / 5.0f };
		float th{ 1.0f / 5.0f };


		SpriteDraw(150.0f - i * 50.0f, 100.0f, 100.0f, 100.0f, tx, ty, tw, th);
	}
}

void AddStroke(int stroke)
{
	g_Stroke += stroke;
}

int GetStroke()
{
	return g_Stroke;
}
