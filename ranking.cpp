#include "ranking.h"

#include "directx.h"
#include "texture.h"
#include "sprite.h"
#include "score.h"

//　アスペクト比は16：9

static constexpr unsigned int rankingMax = 4;

static int g_Texture;
static int g_Score[rankingMax] = { 10, 5, 1, 0};

void SetRankingScore(int score)
{
	g_Score[3] = score;

	for (int i = rankingMax - 1; i > 0; i--)  // rankingMax - 1　これ大事。
	{
		if (g_Score[i] > g_Score[i - 1])
		{
			int score = g_Score[i];
			g_Score[i] = g_Score[i - 1];
			g_Score[i - 1] = score;
		}
	}
}

void InitializeRanking()
{
	g_Texture = TextureLoad(L"asset\\texture\\number_2.png");
}

void FinalizeRanking()
{
}

void UpdateRanking()
{
}

void DrawRanking()
{
	for (int j = 0; j < rankingMax; j++)
	{
		int value = g_Score[j];

		for (int i = 0; i < 5; i++)
		{
			int num = value % 10;
			value /= 10;

			float tx{ (num % 5) / 5.0f };
			float tw{ 1.0f / 5.0f };
			float ty{ (num / 5) / 5.0f };
			float th{ 1.0f / 5.0f };

			SpriteDraw(300.0f - i * 50.0f, 100.0f + j * 100.0f, 100.0f, 100.0f, tx, ty, tw, th,g_Texture);
		}
	}
}