#include "score.h"

#include "directx.h"
#include "texture.h"
#include "sprite.h"

//　アスペクト比は16：9

static int g_Texture;
static int g_Score;

void InitializeScore()
{
	g_Texture = TextureLoad(L"asset\\texture\\number_2.png");

}

void FinalizeScore()
{

}

void UpdateScore()
{

}

void DrawScore()
{
	int value = g_Score;

	for (int i = 0; i < 5; i++)
	{
		int num = value % 10;
		value /= 10;

		float tx{ (num % 5) / 5.0f };
		float tw{ 1.0f / 5.0f };
		float ty{ (num / 5) / 5.0f };
		float th{ 1.0f / 5.0f };

		SpriteDraw(300.0f - i * 50.0f, 100.0f, 100.0f, 100.0f, tx, ty, tw, th,g_Texture);
	}
}

void AddScore(int score)
{
	g_Score += score;
}

int GetScore()
{
	return g_Score;
}