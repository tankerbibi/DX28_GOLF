#include "directX.h"
#include "enemybomb.h"
#include "ball.h"
#include "breakableBlock.h"
#include "camera.h"
#include "effect.h"
#include "field.h"
#include "model.h"
#include "shader.h"
#include "slope.h"
#include "texture.h"
#include "block.h"
#include "player.h"
#include <cmath>
#include <vector>

static int g_Texture = -1;
static ID3D11Buffer* g_VertexBuffer;

static std::vector<EnemyBomb> g_EnemyBombs;

// --- EnemyBomb Class Implementation ---

EnemyBomb::EnemyBomb()
	: position({ 0.0f, 0.0f, 0.0f }), velocity({ 0.0f, 0.0f, 0.0f }),
	rotation({ 0.0f, 0.0f, 0.0f }), state(STATE_INACTIVE), stateCount(0) {
}

void EnemyBomb::Initialize(const DirectX::XMFLOAT3& startPos,
	const DirectX::XMFLOAT3& direction) {
	position = startPos;
	// 正規化
	float speed = firstSpeed;  // スピードを調整
	float len = sqrtf(direction.x * direction.x + direction.y * direction.y +
		direction.z * direction.z);
	if (len > 0.0f) {
		velocity.x = (direction.x / len) * speed;
		velocity.y = (direction.y / len) * speed;
		velocity.z = (direction.z / len) * speed;
	}
	else {
		velocity = { 0.0f, 0.0f, 0.5f }; // Default forward if direction is zero
	}

	// Calculate rotation (Yaw/Pitch) from direction for rendering
	// Simple lookup: Yaw is atan2(x, z)
	rotation.y = atan2f(velocity.x, velocity.z);
	// Pitch is atan2(y, sqrt(x^2 + z^2)) roughly, simplified
	float xzLen = sqrtf(velocity.x * velocity.x + velocity.z * velocity.z);
	rotation.x = -atan2f(velocity.y, xzLen);
	rotation.z = 0.0f;

	state = STATE_START;
	stateCount = 0;

	// トレイルの初期化
	trail.Initialize();
	trail.Reset(position); // 初期位置でリセット
}

void EnemyBomb::Update() {
	if (state == STATE_INACTIVE)
		return;
	static bool trailUpdate = false;
	if (trailUpdate)
	{
		trailUpdate = false;
		trail.Update(position);
	}
	if (!trailUpdate) trailUpdate = true;

	switch (state) {
	case STATE_START:
		stateCount++;
		if (stateCount >
			10)
		{
			state = STATE_MOVE;
			stateCount = 0;
		}
		break;

	case STATE_MOVE:
		Move();
		lifeCount++;
		if (lifeCount > lifeCountMax)
		{
			OnHit();
		}
		if (IsHit())
		{
			OnHit();
		}
		break;

	case STATE_EXPLODED:
		stateCount++;

		if (stateCount == 2) {
		}
		if (stateCount > 60) // End explosion
		{
			state = STATE_INACTIVE;
		}
		break;
	}
}

void EnemyBomb::Draw() {
	if (state == STATE_INACTIVE)
		return; // Don't draw if exploded (effect handles it) or inactive

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

	matrixWorld *= XMMatrixScaling(Radius * 2, Radius * 2, Radius * 2);

	// 回転マトリクス（ビルボード処理）
	matrixWorld *= invView;

	// 移動マトリクス。gpuで計算されている。
	matrixWorld *= XMMatrixTranslation(position.x, position.y, position.z);

	matrix = matrixWorld;

	// ビューマトリクス
	matrix *= GetCameraViewMatrix();

	// プロジェクションマトリクス

	matrix *= GetCameraProjectionMatrix();

	// vertex.hlslのmtxに値を送っている。
	Shader_SetMatrix({ matrix, matrixWorld });

	// ポリゴン描画
	DirectXGetDeviceContext()->Draw(4, 0);
	trail.Draw();

}

void EnemyBomb::Move() {

	// 重力
	velocity.y -= gravity * deltaTime;

	// 抵抗
	velocity.x -= velocity.x * resistance * deltaTime;
	velocity.y -= velocity.y * resistance * deltaTime;
	velocity.z -= velocity.z * resistance * deltaTime;

	// 速度を加算
	position.x += velocity.x * deltaTime;
	position.y += velocity.y * deltaTime;
	position.z += velocity.z * deltaTime;
}

bool EnemyBomb::IsHit() {
	if (ResolveBreakableBlockCollision(position, ExplosionRadius)) {
		return true;
	}

	// 2. Static Blocks との衝突
	BLOCK* block = GetFieldBlock();
	float blockRadius = GetBlockRadius();

	for (int i = 0; i < blockMax; i++) {
		if (block[i].blockType != BLOCKTYPE::BLOCK)
			continue;

		// 【修正点】 position に Radius を考慮する (中心点ではなく球体として判定)

		if (block[i].pos.y - blockRadius < position.y + Radius && // 上端
			position.y - Radius < block[i].pos.y + blockRadius && // 下端
			block[i].pos.z - blockRadius < position.z + Radius && // 奥端
			position.z - Radius < block[i].pos.z + blockRadius && // 手前端
			block[i].pos.x - blockRadius < position.x + Radius && // 右端
			position.x - Radius < block[i].pos.x + blockRadius)   // 左端
		{
			return true;
		}
	}

	// 3. 床（場外）判定
	if (position.y < -10.0f) {
		return true;
	}


	XMFLOAT3 playerPos = GetPlayerPosition();
	// 4. プレイヤーとの衝突判定

	return false;
}

void EnemyBomb::OnHit() {
	state = STATE_EXPLODED;
	stateCount = 0;
}


void EnemyBomb::EnemyBombHitCheck()
{
	{
		Slope* slope = GetSlope();
		float slopeRadius = GetSlopeRadius();

		float e = 0.5f;  // 跳ね返り係数


		for (int i = 0; i < slopeMax; i++)
		{

			float slopeTop = slope[i].position.y + slopeRadius;

			if (slope[i].slopeDirection == SlopeDirection::BACKWARDUP)
				slopeTop = slope[i].position.y + std::min((position.z - slope[i].position.z), slopeRadius);
			else if (slope[i].slopeDirection == SlopeDirection::LEFTUP)
				slopeTop = slope[i].position.y + std::min(-(position.x - slope[i].position.x), slopeRadius);
			else if (slope[i].slopeDirection == SlopeDirection::FORWARDUP)
				slopeTop = slope[i].position.y + std::min(-(position.z - slope[i].position.z), slopeRadius);
			else if (slope[i].slopeDirection == SlopeDirection::RIGHTUP)
				slopeTop = slope[i].position.y + std::min((position.x - slope[i].position.x), slopeRadius);

			// 横方向の当たり判定処理
			if (slope[i].position.y - slopeRadius < position.y &&
				position.y < slopeTop)  // 横からみた図の状況を作り出している！！
			{
				// x方向
				if (slope[i].position.z - slopeRadius < position.z &&
					position.z < slope[i].position.z + slopeRadius)  // 3次元だから2次元に絞ろう！
				{
					if (slope[i].position.x - slopeRadius < position.x + Radius &&
						position.x - Radius < slope[i].position.x + slopeRadius)
					{
						if (slope[i].position.x < position.x)
						{
							// 右
							position.x = slope[i].position.x + slopeRadius + Radius;
						}
						else
						{
							// 左
							position.x = slope[i].position.x - slopeRadius - Radius;
						}
						velocity.x *= -e;
					}
				}
				// z方向
				else if (slope[i].position.x - slopeRadius < position.x + Radius &&
					position.x < slope[i].position.x + slopeRadius)
				{
					if (slope[i].position.z - slopeRadius < position.z + Radius &&
						position.z - Radius < slope[i].position.z + slopeRadius)
					{
						if (slope[i].position.z < position.z)
						{
							// 奥
							position.z = slope[i].position.z + slopeRadius + Radius;
						}
						else
						{
							// 手前
							position.z = slope[i].position.z - slopeRadius - Radius;
						}
					}
				}
			}
			else
				// 縦方向の当たり判定処理
			{
				// 手前　奥
				if (slope[i].position.z - slopeRadius < position.z &&
					position.z < slope[i].position.z + slopeRadius)
				{
					if (slope[i].position.x - slopeRadius < position.x &&
						position.x < slope[i].position.x + slopeRadius)
					{
						if (slope[i].position.y - slopeRadius < position.y + Radius &&
							position.y - Radius < slopeTop)
						{
							if (slopeTop < position.y)
							{
								// 上
								position.y = slopeTop + Radius;

								if (velocity.y < -3.0f)
								{
									CreateEffect(position);
									SetCameraShake(1.0f);
								}

								// 本当は法線から計算してみたいなことをやるらしい。
								float dt = 1.0f / 60.0f;

								if (slope[i].slopeDirection == SlopeDirection::BACKWARDUP)
								{
									// 坂道転がるコード
									velocity.z += -1.0f * dt;
									// バウンドのコード
									velocity.z += -velocity.y * -e;
									velocity.y = 0.0f;
								}
								else if (slope[i].slopeDirection == SlopeDirection::LEFTUP)
								{
									velocity.x += 1.0f * dt;
									velocity.x += velocity.y * -e;
									velocity.y = 0.0f;
								}
								else if (slope[i].slopeDirection == SlopeDirection::FORWARDUP)
								{
									velocity.z += 1.0f * dt;
									velocity.z += velocity.y * -e;
									velocity.y = 0.0f;
								}
								else if (slope[i].slopeDirection == SlopeDirection::RIGHTUP)
								{
									velocity.x += -1.0f * dt;
									velocity.x += -velocity.y * -e;
									velocity.y = 0.0f;
								}
							}
							else
							{
								// 下
								position.y = slope[i].position.y - slopeRadius - Radius;
								velocity.y *= -e;
							}
						}
					}
				}
			}
		}

	}

	{
		BLOCK* block = GetFieldBlock();
		float blockRadius = GetBlockRadius();


		float e = 0.5f;  // 跳ね返り係数

		for (int i = 0; i < blockMax; i++)
		{

			float blockTop = block[i].pos.y + blockRadius;

			// 横方向の当たり判定処理
			if (block[i].pos.y - blockRadius < position.y &&
				position.y < blockTop)  // 横からみた図の状況を作り出している！！
			{
				// x方向
				if (block[i].pos.z - blockRadius < position.z &&
					position.z < block[i].pos.z + blockRadius)  // 3次元だから2次元に絞ろう！
				{
					if (block[i].pos.x - blockRadius < position.x + Radius &&
						position.x - Radius < block[i].pos.x + blockRadius)
					{
						if (block[i].pos.x < position.x)
						{
							// 右
							position.x = block[i].pos.x + blockRadius + Radius;
						}
						else
						{
							// 左
							position.x = block[i].pos.x - blockRadius - Radius;
						}
						velocity.x *= -e;
					}
				}
				// z方向
				else if (block[i].pos.x - blockRadius < position.x + Radius &&
					position.x < block[i].pos.x + blockRadius)
				{
					if (block[i].pos.z - blockRadius < position.z + Radius &&
						position.z - Radius < block[i].pos.z + blockRadius)
					{
						if (block[i].pos.z < position.z)
						{
							// 奥
							position.z = block[i].pos.z + blockRadius + Radius;
						}
						else
						{
							// 手前
							position.z = block[i].pos.z - blockRadius - Radius;
						}
					}
				}
			}
			else
				// 縦方向の当たり判定処理
			{
				// 手前　奥
				if (block[i].pos.z - blockRadius < position.z &&
					position.z < block[i].pos.z + blockRadius)
				{
					if (block[i].pos.x - blockRadius < position.x &&
						position.x < block[i].pos.x + blockRadius)
					{
						if (block[i].pos.y - blockRadius < position.y + Radius &&
							position.y - Radius < blockTop)
						{
							if (position.y > block[i].pos.y)
							{
								// 上
								position.y = blockTop + Radius;

								if (velocity.y < -3.0f)
								{
									CreateEffect(position);
									SetCameraShake(1.0f);
								}
							}
							else
							{
								// 下
								position.y = block[i].pos.y - blockRadius - Radius;
							}
							velocity.y *= -e;
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
			if (breakableBlock[i].position.y - blockRadius < position.y &&
				position.y < breakableBlock[i].position.y + blockRadius)  // 横からみた図の状況を作り出している！！
			{
				// x方向
				if (breakableBlock[i].position.z - blockRadius < position.z &&
					position.z < breakableBlock[i].position.z + blockRadius)  // 3次元だから2次元に絞ろう！
				{
					if (breakableBlock[i].position.x - blockRadius < position.x + Radius &&
						position.x - Radius < breakableBlock[i].position.x + blockRadius)
					{
						if (breakableBlock[i].position.x < position.x)
						{
							// 右
							position.x = breakableBlock[i].position.x + blockRadius + Radius;
						}
						else
						{
							// 左
							position.x = breakableBlock[i].position.x - blockRadius - Radius;
						}
						velocity.x *= -e;
					}
				}
				// z方向
				else if (breakableBlock[i].position.x - blockRadius < position.x + Radius &&
					position.x < breakableBlock[i].position.x + blockRadius)
				{
					if (breakableBlock[i].position.z - blockRadius < position.z + Radius &&
						position.z - Radius < breakableBlock[i].position.z + blockRadius)
					{
						if (breakableBlock[i].position.z < position.z)
						{
							// 奥
							position.z = breakableBlock[i].position.z + blockRadius + Radius;
						}
						else
						{
							// 手前
							position.z = breakableBlock[i].position.z - blockRadius - Radius;
						}
					}
				}
			}
			else
				// 縦方向の当たり判定処理
			{
				// 手前　奥
				if (breakableBlock[i].position.z - blockRadius < position.z &&
					position.z < breakableBlock[i].position.z + blockRadius)
				{
					if (breakableBlock[i].position.x - blockRadius < position.x &&
						position.x < breakableBlock[i].position.x + blockRadius)
					{
						if (breakableBlock[i].position.y - blockRadius < position.y + Radius &&
							position.y - Radius < breakableBlock[i].position.y + blockRadius)
						{
							if (position.y > breakableBlock[i].position.y)
							{
								// 上
								position.y = breakableBlock[i].position.y + blockRadius + Radius;

								if (velocity.y < -3.0f)
								{
									CreateEffect(position);
									SetCameraShake(1.0f);
								}
							}
							else
							{
								// 下
								position.y = breakableBlock[i].position.y - blockRadius - Radius;
							}
							velocity.y *= -e;
						}
					}
				}
			}
		}
	}
}

// --- Global Management Functions ---

void InitializeEnemyBomb() {

	// ゲーム初期化時
	Trail::LoadCommonResources();


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

	g_Texture = TextureLoad(L"asset\\texture\\crystalBall_black.png");
	g_EnemyBombs.clear();
}

void FinalizeEnemyBomb() {
	g_EnemyBombs.clear();
	// ゲーム終了時
	Trail::UnloadCommonResources();
}

void UpdateEnemyBomb() {
	// Update all enemybombs
	for (auto it = g_EnemyBombs.begin(); it != g_EnemyBombs.end();) {
		it->Update();
		if (!it->IsActive()) {
			it = g_EnemyBombs.erase(it);
		}
		else {
			it++;
		}
	}
}

void DrawEnemyBomb() {
	for (auto& enemybomb : g_EnemyBombs) {
		enemybomb.Draw();
	}
}

void CreateEnemyBomb(const DirectX::XMFLOAT3& position,
	const DirectX::XMFLOAT3& direction) {
	EnemyBomb newEnemyBomb;
	newEnemyBomb.Initialize(position, direction);
	g_EnemyBombs.push_back(newEnemyBomb);
}

DirectX::XMFLOAT3 GetEnemyBombPos() {
	if (!g_EnemyBombs.empty())
		return g_EnemyBombs.back().GetPosition();
	return { 0.0f, 0.0f, 0.0f };
}

void SetEnemyBombStartPosition(DirectX::XMFLOAT3 newPosition) {}
