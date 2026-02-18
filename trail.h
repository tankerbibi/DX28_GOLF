#pragma once

void InitializeTrail();
void FinalizeTrail();
void UpdateTrail();
void DrawTrail();

void SetTrailPosition(XMFLOAT3 position);
void ResetTrailPosition(XMFLOAT3 position);


////////////////////////////////////////////////////
using namespace DirectX;

class Trail
{
public:
	// コンストラクタ・デストラクタ
	Trail();
	~Trail();

	// 初期化（個別の頂点バッファ作成）
	void Initialize();

	// 終了処理（バッファ開放）
	void Finalize();

	// 更新（新しい座標を追加）
	void Update(XMFLOAT3 position);

	// 描画
	void Draw();

	// 位置のリセット（出現時などに使用）
	void Reset(XMFLOAT3 position);

	// --- 静的メンバ（全トレイル共通のリソース管理） ---
	static void LoadCommonResources();   // テクスチャ読み込み
	static void UnloadCommonResources(); // テクスチャ破棄

private:
	// 定数：頂点数
	static const int TRAIL_LENGTH = 30;

	// メンバ変数：このトレイル専用の頂点バッファ
	ID3D11Buffer* m_VertexBuffer = nullptr;

	// メンバ変数：このトレイルの座標履歴
	XMFLOAT3 m_History[TRAIL_LENGTH];

	// 静的メンバ：共通テクスチャID
	static int s_Texture;
};