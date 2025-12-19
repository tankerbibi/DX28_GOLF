#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN //無駄な機能を省くためのやつ
#include <Windows.h>
#include <mmsystem.h>
#include "main.h"
#include "directx.h"
#include "shader.h"
#include "texture.h"
#include "sprite.h"
#include "Keyboard.h"
#include "game.h"
#include "title.h"
#include "result.h"
#include "sound.h"

#pragma comment(lib, "winmm.lib")

#ifdef _DEBUG
int g_CountFPS;
char g_DebugStr[2048];
#endif


void Initialize(HWND hWnd);
void Finalize();
void Update();
void Draw();
void ChangeFullWindow();
HWND GetWindow();

static constexpr char WINDOW_CLASS[] = "GameWindow";
static constexpr char TITLE[] = "ウィンドウ表示";
static bool g_FullWindow{false};
HWND g_HWnd;
int g_WindowWidth;
int g_WindowHeight;

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM iPara);

static SCENE g_Scene;


int APIENTRY WinMain(_In_ HINSTANCE hInstance,  // １年生の時に作っていたmain関数にあたるもの　エントリーポイント
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR LpCmdLine, _In_ int nCmdShow)
{
	/*ウィンドウクラスの登録*/
	WNDCLASSEX wcex{};
	wcex.cbSize = sizeof(WNDCLASSEXW);
	wcex.lpfnWndProc = WndProc;  // 関数ポインタ　関数を登録した
	wcex.hInstance = hInstance;
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	//wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.hbrBackground = NULL;
	wcex.lpszMenuName = nullptr;  // NULLよりも後の時代に登場したのは、nullPtr。nullそのものを表している。
	wcex.lpszClassName = WINDOW_CLASS;
	wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);
	RegisterClassEx(&wcex);

	// クライアント領域のサイズを持った短径(左からleft, top, right , bottom)
	RECT window_rect = { 0, 0, screenWidth, screenHeight };  // 直値を入れるのはあんまりよくないコード。defineや定数でやるべき。
	// ウィンドウのスタイル
	 DWORD window_style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
	// 指定したクライアント領域を確保するために新たな矩形座標を計算　ゲームを表示する領域
	AdjustWindowRect(&window_rect, WS_OVERLAPPEDWINDOW, FALSE);

	g_WindowWidth = window_rect.right - window_rect.left;
	g_WindowHeight = window_rect.bottom - window_rect.top;

	/*メインウィンドウの作成*/
	g_HWnd = CreateWindow(WINDOW_CLASS, TITLE, window_style,  // WSはウィンドウスタイル window style。
		CW_USEDEFAULT, CW_USEDEFAULT, g_WindowWidth, g_WindowHeight, nullptr, nullptr, hInstance, nullptr);  // タイトルバーを含めた大きさでやられてしまっている

	Initialize(g_HWnd);

	ShowWindow(g_HWnd, SW_SHOW /*nCmdShow*/);
	UpdateWindow(g_HWnd);


	LARGE_INTEGER queryPerformanceFrequency;
	QueryPerformanceFrequency(&queryPerformanceFrequency);

	LARGE_INTEGER queryPerformanceCounter;
	QueryPerformanceCounter(&queryPerformanceCounter);
	// LONGLONG lastTime = queryPerformanceCounter.QuadPart;

	LARGE_INTEGER freq, lastTime;
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&lastTime);

	double targetFrameTime = 1.0 / 60.0;
	double accumulator = 0.0;

	int dwFrameCount = 0;
	/*メッセージループ*/ //プレイヤーの入力もメッセージである。
	MSG msg;

	DWORD dwCurrentTime;
	DWORD dwExecLastFpsUpdateTime = 0;

	do {
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			LARGE_INTEGER currentTime;
			QueryPerformanceCounter(&currentTime);

			double deltaTime = static_cast<double>(currentTime.QuadPart - lastTime.QuadPart) / freq.QuadPart;
			lastTime = currentTime;

			accumulator += deltaTime;

			if (accumulator >= targetFrameTime)
			{
				Update();
				Draw();

				accumulator = 0.0;

				dwFrameCount++;
			}


			dwCurrentTime = timeGetTime();

			if ((dwCurrentTime - dwExecLastFpsUpdateTime) >= 1000)
			{
				dwExecLastFpsUpdateTime = dwCurrentTime;
				g_CountFPS = dwFrameCount;
				dwExecLastFpsUpdateTime = dwCurrentTime;
				dwFrameCount = 0;
#ifdef _DEBUG
				wsprintf(g_DebugStr, TITLE);
				wsprintf(&g_DebugStr[strlen(g_DebugStr)], "FPS:%d", g_CountFPS);
				SetWindowText(g_HWnd, g_DebugStr);  // 途中でタイトルバーを変えることができるよ！
#endif
			}

		}
	} while (msg.message != WM_QUIT);

	Finalize();

	return (int)msg.wParam;
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)  // ウィンドウプロシージャ　イベントドリブン　メッセージを処理するため
{
	switch (message)
	{
	case WM_DESTROY: //ウィンドウの破棄メッセージ
		PostQuitMessage(0); // WN_QUITメッセージの送信
		break;

	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_ESCAPE:
			if (MessageBox(NULL, "アプリを終了しますか?", "終了シーケンス", MB_YESNO | MB_ICONINFORMATION) == IDYES) //ビットフラグ　複合　複数の設定の引数
			{
				SendMessage(hWnd, WM_CLOSE, 0, 0); //いろんなことができる関数
				DestroyWindow(hWnd);
			}
			break;
		case VK_F11:
			ChangeFullWindow();
			break;
		}
	case WM_SYSKEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYUP: //  キー入力を使いたい場合、個々のケース文が必要になることがある。
		Keyboard_ProcessMessage(message, wParam, lParam); //  いったん入力状態をバッファに保存しておいて、後でそのデータを扱うという考え方。
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam); //  通常のメッセージ処理はこの関数に任せる
	}
}

void Update()
{
	switch (g_Scene)
	{
	case SCENE_TITLE:
		UpdateTitle();
		break;
	case SCENE_GAME:
		UpdateGame();
		break;
	case SCENE_RESULT:
		UpdateResult();
		break;
	default:
		break;
	}
	
	Keyboard_Update();  // 過去の状態を比較しなければならないので、一番最後に置く。
}

void Draw()
{
	Clear(); //画面きれいに

	switch (g_Scene)
	{
	case SCENE_TITLE:
		DrawTitle();
		break;
	case SCENE_GAME:
		DrawGame();
		break;
	case SCENE_RESULT:
		DrawResult();
		break;
	default:
		break;
	}

	Present(); //切り替え
}

void Initialize(HWND hWnd)
{
	//comの初期化
	(void)CoInitializeEx(nullptr, COINITBASE_MULTITHREADED);

	Keyboard_Initialize();

	InitSound(hWnd);

	//DirectX初期化
	DirectXInitialize(hWnd);

	//シェーダー初期化
	Shader_Initialize(DirectXGetDevice(),
		DirectXGetDeviceContext());
	//テクスチャ初期化
	TextureInitialize(DirectXGetDevice());

	Shader_Begin();
	//スプライト初期化
	SpriteInitialize();

	g_Scene = SCENE_GAME;

	SetScene(g_Scene);

	ChangeFullWindow();
}

void Finalize()
{
	SetScene(SCENE_NONE);

	// スプライト終了処理
	SpriteFinalize();

	// テクスチャ終了処理
	TextureFinalize();

	// シェーダー終了処理
	Shader_Finalize();

	// DirectX終了処理
	DirectXFinalize();

	UninitSound();
}

void ChangeFullWindow()
{
	g_FullWindow = !g_FullWindow;

	if (g_FullWindow)
	{
		SetWindowLong(g_HWnd, GWL_STYLE, WS_POPUP);
		//SetFullScreen(g_FullWindow);
		ShowWindow(g_HWnd, SW_MAXIMIZE);
	}
	else
	{
		SetWindowLong(g_HWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);
		//SetFullScreen(g_FullWindow);
		SetWindowPos(g_HWnd, NULL, 0, 0, g_WindowWidth, g_WindowHeight, (SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED));
		ShowWindow(g_HWnd, SW_NORMAL);
	}
}

HWND GetWindow()
{
	return g_HWnd;
}

void SetScene(SCENE scene)
{

	g_Scene = scene;

	switch (g_Scene)
	{
	case SCENE_TITLE:
		InitializeTitle();
		break;
	case SCENE_GAME:
		InitializeGame();
		break;
	case SCENE_RESULT:
		InitializeResult();
		break;
	default:
		break;
	}
}