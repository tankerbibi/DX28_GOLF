#include "directX.h"
#include "ball.h"
#include "Keyboard.h"
#include "model.h"
#include "camera.h"
#include "shader.h"
#include "field.h"
#include "goal.h"
#include "main.h"
#include "stroke.h"
#include "ranking.h"
#include "effect.h"
#include "trail.h"
#include "shadow.h"
#include "breakableBlock.h"
#include "texture.h"
#include "start.h"
#include "slope.h"

static MODEL* g_Model = nullptr;
static int g_Texture = -1;

//static int g_TrailId;

static XMFLOAT3 g_Position;
static XMFLOAT3 g_Velocity;
static XMFLOAT3 g_Rotation;
// 外部的な力
static XMFLOAT3 g_OutForce;

enum BALL_STATE
{
	BALL_STATE_START,
	BALL_STATE_MOVE,
	BALL_STATE_GOAL,
};
static BALL_STATE g_State;
static int g_StateCount;

static ID3D11Buffer* g_VertexBuffer;

static constexpr float g_BallRadius = 1.0f;

void BallHitCheck();
void MoveBall();

void InitializeBall()
{

	g_Model = ModelLoad("asset\\model\\ball.fbx");
	g_Texture = TextureLoad(L"asset\\texture\\WalkRight.png");
	g_Position = {0.0f, 0.0f, 0.0f};
	g_Rotation = { 0.0f, 0.0f, 0.0f };
	g_Velocity = { 0.0f, 0.0f ,0.0f };
	g_State = BALL_STATE_START;
	g_StateCount = 0;

	// ResetTrailPosition(g_Position);

	{
		// 頂点バッファの作成
		D3D11_BUFFER_DESC bd{};
		bd.Usage = D3D11_USAGE_DYNAMIC;
		// 4頂点分
		bd.ByteWidth = sizeof(Vertex) * 4;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		DirectXGetDevice()->CreateBuffer(&bd, nullptr, &g_VertexBuffer);
	}

	///////////////////頂点バッファ設定開始///////////////////////
	{
		D3D11_MAPPED_SUBRESOURCE msr;
		DirectXGetDeviceContext()->Map(g_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr); //g_VertexBufferのありかを探す。

		Vertex* v = (Vertex*)msr.pData;

		// 3dにおいては、ピクセル単位ではなくメートル単位になる。
		// xが横方向、yが縦方向、zが奥行方向
		//四つの頂点を作る。頭の中でイメージ

		float tx = 0.0;
		float ty = 0.0f;
		float tw = 1.0f;
		float th = 1.0f;

		v[0].position = { -0.5f, 0.5f, 0.0f };
		v[1].position = { 0.5f, 0.5f, 0.0f };
		v[2].position = { -0.5f, -0.5f, 0.0f };
		v[3].position = { 0.5f, -0.5f, 0.0f };

		v[0].texcoord = { tx,		ty };
		v[1].texcoord = { tx + tw,	ty };
		v[2].texcoord = { tx,		ty + th };
		v[3].texcoord = { tx + tw,	ty + th };

		v[0].normal = { 0.0f, 0.0f, -1.0f };
		v[1].normal = { 0.0f, 0.0f, -1.0f };
		v[2].normal = { 0.0f, 0.0f, -1.0f };
		v[3].normal = { 0.0f, 0.0f, -1.0f };

		DirectXGetDeviceContext()->Unmap(g_VertexBuffer, 0);
	}
	//////////////頂点バッファ設定終了////////////////////
}

void UpdateBall()
{
	// ステートマシン ステートパターンというものもあるらしい。
	switch (g_State)
	{
	case BALL_STATE_START:
		g_Position = GetStartPosition();
		g_Rotation = { 0.0f, 0.0f, 0.0f };
		g_Velocity = { 0.0f, 0.0f ,0.0f };
		g_State = BALL_STATE_MOVE;
		break;
	case BALL_STATE_MOVE:
		MoveBall();
		break;
	case BALL_STATE_GOAL:
		g_StateCount++;
		if (g_StateCount > 60)
		{
			SetScene(SCENE_RESULT);
		}
		break;
	default:
		break;
	}
}

void DrawBall()
{
	// 頂点バッファ設定
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	DirectXGetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset); //気を付けて　GetじゃなくてSet

	// プリミティブトポロジ設定
	DirectXGetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);  // トライアングルストリップ（連続） つまりZの書き方

	ID3D11ShaderResourceView* texture = GetTexture(g_Texture);
	DirectXGetDeviceContext()->PSSetShaderResources(0, 1, &texture);

	// ビューマトリクスを取得 カメラのビューマトリクスはカメラが向いている方向そのもの。
	XMMATRIX view = GetCameraViewMatrix();
	// ビューマトリクスの逆行列を求める 掛け算の代わりに割り算をするみたいな感じ。
	XMMATRIX invView = XMMatrixInverse(nullptr, view);

	// 移動成分を消去
	invView.r[3].m128_f32[0] = 0.0f;
	invView.r[3].m128_f32[1] = 0.0f;
	invView.r[3].m128_f32[2] = 0.0f;

	// 頂点シェーダーに変換行列を設定
	XMMATRIX matrix = XMMatrixIdentity();  // 行列を作成　float 4 x 4
	XMMATRIX matrixWorld = XMMatrixIdentity();  // 行列を作成　float 4 x 4

	matrixWorld *= XMMatrixScaling(2.5f, 2.5f, 2.5f);

	// 回転マトリクス（ビルボード処理）
	matrixWorld *= invView;

	// 移動マトリクス。gpuで計算されている。
	matrixWorld *= XMMatrixTranslation(g_Position.x, g_Position.y, g_Position.z);

	matrix = matrixWorld;

	// ビューマトリクス
	matrix *= GetCameraViewMatrix();

	// プロジェクションマトリクス

	matrix *= GetCameraProjectionMatrix();

	// vertex.hlslのmtxに値を送っている。
	Shader_SetMatrix({ matrix, matrixWorld });

	// ポリゴン描画
	DirectXGetDeviceContext()->Draw(4, 0);

	// ModelDraw(g_Model);
}

void FinalizeBall()
{
	ModelRelease(g_Model);
	SAFE_RELEASE(g_VertexBuffer);
}


void MoveBall()
{
	XMFLOAT3 cameraForward = GetCameraForward();

	// y成分を消す
	cameraForward.y = 0.0f;

	// sqrtfはルート 三平方の定理
	float length = sqrtf(cameraForward.x * cameraForward.x
						+ cameraForward.y * cameraForward.y 
						+ cameraForward.z * cameraForward.z);

	// Normalize 正規化(長さを１にする) 長さ３のベクトルを長さ３で割るので１になる
	cameraForward.x /= length;
	cameraForward.y /= length;
	cameraForward.z /= length;
	
	// 力
	XMFLOAT3 force = { 0.0f, 0.0f, 0.0f };

	// 加速
	CameraMode cameraMode = GetCameraMode();
	if (cameraMode == CameraMode::PLAY)
	{
		if (Keyboard_IsKeyDown(KK_A))
		{
			// 例のベクトルの回転の公式
			force.x -= cameraForward.z; // xとzを返ればいいだけ
			force.z += cameraForward.x;

		}
		else if (Keyboard_IsKeyDown(KK_D))
		{
			force.x += cameraForward.z;
			force.z -= cameraForward.x;
		}
		if (Keyboard_IsKeyDown(KK_W))
		{
			force.x += cameraForward.x;
			force.z += cameraForward.z;
		}
		else if (Keyboard_IsKeyDown(KK_S))
		{
			force.x -= cameraForward.x;
			force.z -= cameraForward.z;
		}
	}

	// 力ベクトルの長さ
	float forceLength = sqrtf(force.x * force.x
		+ force.y * force.y
		+ force.z * force.z);

	// 正規化（長さを１にする）
	if (forceLength > 1.0f)  // じゃあ、長さが１以下のときはどうなる？
	{
		force.x /= forceLength;
		force.y /= forceLength;
		force.z /= forceLength;
	}

	static constexpr float forcePower = 10.0f;

	g_Velocity.x += force.x * forcePower * deltaTime;
	g_Velocity.z += force.z * forcePower * deltaTime;

	static constexpr float shotPower = 5.0f;

	// ショット
	if (Keyboard_IsKeyTrigger(KK_SPACE))
	{
		g_Velocity.x += cameraForward.x * shotPower;
		g_Velocity.z += cameraForward.z * shotPower;

		g_Velocity.y += shotPower;  // 撃力

		// 打数加算
		AddStroke(1);
		
		SetRankingScore(GetStroke());

		CreateEffect(g_Position);
	}

	// 重力
	g_Velocity.y -= 9.8f * deltaTime;

	// 抵抗
	g_Velocity.x -= g_Velocity.x * 1.0f * deltaTime;
	g_Velocity.y -= g_Velocity.y * 1.0f * deltaTime;
	g_Velocity.z -= g_Velocity.z * 1.0f * deltaTime;

	// 移動
	g_Position.x += g_Velocity.x * deltaTime;
	g_Position.y += g_Velocity.y * deltaTime;
	g_Position.z += g_Velocity.z * deltaTime;

	if (g_Position.y < -7)
	{
		g_State = BALL_STATE_START;
	}

	// 衝突判定
	BallHitCheck();

	//SetTrailPosition(g_Position);

	SetShadowPosition(g_Position);

	// 終点から始点を引くことで、ベクトルを求める
	XMFLOAT3 goalVec;
	XMFLOAT3 goalPosition = GetGoalPosition();
	goalVec.x = goalPosition.x - g_Position.x;
	goalVec.y = goalPosition.y - g_Position.y;
	goalVec.z = goalPosition.z - g_Position.z;

	float goalLength = sqrtf(goalVec.x * goalVec.x
		+ goalVec.y * goalVec.y
		+ goalVec.z * goalVec.z);

	// ゴール衝突判定
	if (goalLength < g_BallRadius * 4.0f)
	{
		// SetRankingScore(GetStroke());

		g_State = BALL_STATE_GOAL;
		g_StateCount = 0;

		SetGoalShake(3.0f);
	}
}

XMFLOAT3 GetBallPosition()
{
	return g_Position;
}

void BallHitCheck()
{
	{
		Slope* slope = GetSlope();
		float slopeRadius = 1.5f;


		float e = 0.5f;  // 跳ね返り係数

		for (int i = 0; i < slopeMax; i++)
		{

			float slopeTop = slope[i].position.y + slopeRadius;

			if (slope[i].slopeDirection == SlopeDirection::BACKWARDUP)
				slopeTop = slope[i].position.y + std::min((g_Position.z - slope[i].position.z), slopeRadius);
			else if (slope[i].slopeDirection == SlopeDirection::LEFTUP)
				slopeTop = slope[i].position.y + std::min(-(g_Position.x - slope[i].position.x), slopeRadius);
			else if (slope[i].slopeDirection == SlopeDirection::FORWARDUP)
				slopeTop = slope[i].position.y + std::min(-(g_Position.z - slope[i].position.z), slopeRadius);
			else if (slope[i].slopeDirection == SlopeDirection::RIGHTUP)
				slopeTop = slope[i].position.y + std::min((g_Position.x - slope[i].position.x), slopeRadius);

			// 横方向の当たり判定処理
			if (slope[i].position.y - slopeRadius < g_Position.y &&
				g_Position.y < slopeTop)  // 横からみた図の状況を作り出している！！
			{
				// x方向
				if (slope[i].position.z - slopeRadius < g_Position.z &&
					g_Position.z < slope[i].position.z + slopeRadius)  // 3次元だから2次元に絞ろう！
				{
					if (slope[i].position.x - slopeRadius < g_Position.x + g_BallRadius &&
						g_Position.x - g_BallRadius < slope[i].position.x + slopeRadius)
					{
						if (slope[i].position.x < g_Position.x)
						{
							// 右
							g_Position.x = slope[i].position.x + slopeRadius + g_BallRadius;
						}
						else
						{
							// 左
							g_Position.x = slope[i].position.x - slopeRadius - g_BallRadius;
						}
						g_Velocity.x *= -e;
					}
				}
				// z方向
				else if (slope[i].position.x - slopeRadius < g_Position.x + g_BallRadius &&
					g_Position.x < slope[i].position.x + slopeRadius)
				{
					if (slope[i].position.z - slopeRadius < g_Position.z + g_BallRadius &&
						g_Position.z - g_BallRadius < slope[i].position.z + slopeRadius)
					{
						if (slope[i].position.z < g_Position.z)
						{
							// 奥
							g_Position.z = slope[i].position.z + slopeRadius + g_BallRadius;
						}
						else
						{
							// 手前
							g_Position.z = slope[i].position.z - slopeRadius - g_BallRadius;
						}
					}
				}
			}
			else
				// 縦方向の当たり判定処理
			{
				// 手前　奥
				if (slope[i].position.z - slopeRadius < g_Position.z &&
					g_Position.z < slope[i].position.z + slopeRadius)
				{
					if (slope[i].position.x - slopeRadius < g_Position.x &&
						g_Position.x < slope[i].position.x + slopeRadius)
					{
						if (slope[i].position.y - slopeRadius < g_Position.y + g_BallRadius &&
							g_Position.y - g_BallRadius < slopeTop)
						{
							if (slopeTop < g_Position.y )
							{
								// 上
								g_Position.y = slopeTop + g_BallRadius;

								if (g_Velocity.y < -3.0f)
								{
									CreateEffect(g_Position);
									SetCameraShake(1.0f);
								}

								// 本当は法線から計算してみたいなことをやるらしい。
								float dt = 1.0f / 60.0f;

								if (slope[i].slopeDirection == SlopeDirection::BACKWARDUP)
								{
									// 坂道転がるコード
									g_Velocity.z += -1.0f * dt;
									// バウンドのコード
									g_Velocity.z += -g_Velocity.y * -e;
									g_Velocity.y = 0.0f;
								}
								else if (slope[i].slopeDirection == SlopeDirection::LEFTUP)
								{
									g_Velocity.x += 1.0f * dt;
									g_Velocity.x += g_Velocity.y * -e;
									g_Velocity.y = 0.0f;
								}
								else if (slope[i].slopeDirection == SlopeDirection::FORWARDUP)
								{
									g_Velocity.z += 1.0f * dt;
									g_Velocity.z += g_Velocity.y * -e;
									g_Velocity.y = 0.0f;
								}
								else if (slope[i].slopeDirection == SlopeDirection::RIGHTUP)
								{
									g_Velocity.x += -1.0f * dt;
									g_Velocity.x += -g_Velocity.y * -e;
									g_Velocity.y = 0.0f;
								}
							}
							else
							{
								// 下
								g_Position.y = slope[i].position.y - slopeRadius - g_BallRadius;
								g_Velocity.y *= -e;
							}
						}
					}
				}
			}
		}
	
	}

	{
		BLOCK* block = GetFieldBlock();
		float blockRadius = 1.5f;


		float e = 0.5f;  // 跳ね返り係数

		for (int i = 0; i < blockMax; i++)
		{

			float blockTop = block[i].pos.y + blockRadius;

			// 横方向の当たり判定処理
			if (block[i].pos.y - blockRadius < g_Position.y &&
				g_Position.y < blockTop)  // 横からみた図の状況を作り出している！！
			{
				// x方向
				if (block[i].pos.z - blockRadius < g_Position.z &&
					g_Position.z < block[i].pos.z + blockRadius)  // 3次元だから2次元に絞ろう！
				{
					if (block[i].pos.x - blockRadius < g_Position.x + g_BallRadius &&
						g_Position.x - g_BallRadius < block[i].pos.x + blockRadius)
					{
						if (block[i].pos.x < g_Position.x)
						{
							// 右
							g_Position.x = block[i].pos.x + blockRadius + g_BallRadius;
						}
						else
						{
							// 左
							g_Position.x = block[i].pos.x - blockRadius - g_BallRadius;
						}
						g_Velocity.x *= -e;
					}
				}
				// z方向
				else if (block[i].pos.x - blockRadius < g_Position.x + g_BallRadius &&
					g_Position.x < block[i].pos.x + blockRadius)
				{
					if (block[i].pos.z - blockRadius < g_Position.z + g_BallRadius &&
						g_Position.z - g_BallRadius < block[i].pos.z + blockRadius)
					{
						if (block[i].pos.z < g_Position.z)
						{
							// 奥
							g_Position.z = block[i].pos.z + blockRadius + g_BallRadius;
						}
						else
						{
							// 手前
							g_Position.z = block[i].pos.z - blockRadius - g_BallRadius;
						}
					}
				}
			}
			else
				// 縦方向の当たり判定処理
			{
				// 手前　奥
				if (block[i].pos.z - blockRadius < g_Position.z &&
					g_Position.z < block[i].pos.z + blockRadius)
				{
					if (block[i].pos.x - blockRadius < g_Position.x &&
						g_Position.x < block[i].pos.x + blockRadius)
					{
						if (block[i].pos.y - blockRadius < g_Position.y + g_BallRadius &&
							g_Position.y - g_BallRadius < blockTop)
						{
							if (g_Position.y > block[i].pos.y)
							{
								// 上
								g_Position.y = blockTop + g_BallRadius;

								if (g_Velocity.y < -3.0f)
								{
									CreateEffect(g_Position);
									SetCameraShake(1.0f);
								}
							}
							else
							{
								// 下
								g_Position.y = block[i].pos.y - blockRadius - g_BallRadius;
							}
							g_Velocity.y *= -e;
						}
					}
				}
			}
		}
	}
	{
		BreakableBlock* breakableBlock = GetBreakableBlock();

		float blockRadius = 0.7f;


		float e = 0.5f;  // 跳ね返り係数
		for (int i = 0; i < maxBreakableBlock; i++)
		{
			if (breakableBlock[i].use == false) continue;
			// 横方向の当たり判定処理
			if (breakableBlock[i].position.y - blockRadius < g_Position.y &&
				g_Position.y < breakableBlock[i].position.y + blockRadius)  // 横からみた図の状況を作り出している！！
			{
				// x方向
				if (breakableBlock[i].position.z - blockRadius < g_Position.z &&
					g_Position.z < breakableBlock[i].position.z + blockRadius)  // 3次元だから2次元に絞ろう！
				{
					if (breakableBlock[i].position.x - blockRadius < g_Position.x + g_BallRadius &&
						g_Position.x - g_BallRadius < breakableBlock[i].position.x + blockRadius)
					{
						if (breakableBlock[i].position.x < g_Position.x)
						{
							// 右
							g_Position.x = breakableBlock[i].position.x + blockRadius + g_BallRadius;
						}
						else
						{
							// 左
							g_Position.x = breakableBlock[i].position.x - blockRadius - g_BallRadius;
						}
						g_Velocity.x *= -e;
					}
				}
				// z方向
				else if (breakableBlock[i].position.x - blockRadius < g_Position.x + g_BallRadius &&
					g_Position.x < breakableBlock[i].position.x + blockRadius)
				{
					if (breakableBlock[i].position.z - blockRadius < g_Position.z + g_BallRadius &&
						g_Position.z - g_BallRadius < breakableBlock[i].position.z + blockRadius)
					{
						if (breakableBlock[i].position.z < g_Position.z)
						{
							// 奥
							g_Position.z = breakableBlock[i].position.z + blockRadius + g_BallRadius;
						}
						else
						{
							// 手前
							g_Position.z = breakableBlock[i].position.z - blockRadius - g_BallRadius;
						}
					}
				}
			}
			else
				// 縦方向の当たり判定処理
			{
				// 手前　奥
				if (breakableBlock[i].position.z - blockRadius < g_Position.z &&
					g_Position.z < breakableBlock[i].position.z + blockRadius)
				{
					if (breakableBlock[i].position.x - blockRadius < g_Position.x &&
						g_Position.x < breakableBlock[i].position.x + blockRadius)
					{
						if (breakableBlock[i].position.y - blockRadius < g_Position.y + g_BallRadius &&
							g_Position.y - g_BallRadius < breakableBlock[i].position.y + blockRadius)
						{
							if (g_Position.y > breakableBlock[i].position.y)
							{
								// 上
								g_Position.y = breakableBlock[i].position.y + blockRadius + g_BallRadius;

								if (g_Velocity.y < -3.0f)
								{
									CreateEffect(g_Position);
									SetCameraShake(1.0f);
								}
							}
							else
							{
								// 下
								g_Position.y = breakableBlock[i].position.y - blockRadius - g_BallRadius;
							}
							g_Velocity.y *= -e;
						}
					}
				}
			}
		}
	}

}

void AddForce(XMFLOAT3 force)
{
	g_Velocity.x += force.x;
	g_Velocity.y += force.y;
	g_Velocity.z += force.z;
}
