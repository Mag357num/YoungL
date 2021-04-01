#include "pch.h"
#include "GameCore.h"
#include "Misc/GameInput.h"
#include "Misc/GameTimer.h"
#include "Graphics/Buffer/ColorBuffer.h"
#include "Graphics/GraphicsCore.h"
#include "Graphics/RHI/Display.h"

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
#pragma comment(lib, "runtimeobject.lib")
#else
#include <agile.h>
	using namespace Windows::ApplicationModel;
	using namespace Windows::UI::Core;
	using namespace Windows::UI::ViewManagement;
	using Windows::ApplicationModel::Core::CoreApplication;
	using Windows::ApplicationModel::Core::CoreApplicationView;
	using Windows::ApplicationModel::Activation::IActivatedEventArgs;
	using Windows::Foundation::TypedEventHandler;
#endif

namespace Graphics
{
	extern FColorBuffer g_GenMipsBuffer;
}

namespace GameCore
{
	void InitializeApplication(IGameApp& game)
	{
		Graphics::Initialize();
		//SystemTime::Initialize();
		//GameInput::Initialize();
		//EngineTuning::Initialize();

		game.Startup();
	}

	void TerminateApplication(IGameApp& game)
	{
		Graphics::g_CommandManager.IdleGPU();
		game.Cleanup();

		//GameInput::ShutDown();
	}

	bool UpdateApplication(IGameApp& game)
	{
		//EngineProfiling::Update();

		float DeltaTime = Graphics::GetFrameTime();

		//GameInput::Update(DeltaTime);
		//EngineTuning::Update(DeltaTime);

		game.Update(DeltaTime);
		game.RenderScene();

		Display::Present();

		return !game.IsDone();
	}

	// Default implementation to be overridden by the application
	bool IGameApp::IsDone(void)
	{
		//return GameInput::IsFirstPressed(GameInput::kKey_escape);
		return false;
	}

#if !WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)

#else // Win32
	HWND g_hWnd = nullptr;

	//void InitWindow(const wchar_t* className);
	LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

	void RunApplication(IGameApp& app, const wchar_t* className, HINSTANCE hInst, int nCmdShow)
	{
		//ASSERT_SUCCEEDED(CoInitializeEx(nullptr, COINITBASE_MULTITHREADED));
		Microsoft::WRL::Wrappers::RoInitializeWrapper InitializeWinRT(RO_INIT_MULTITHREADED);
		ASSERT_SUCCEEDED(InitializeWinRT);

		//HINSTANCE hInst = GetModuleHandle(0);

		// Register class
		WNDCLASSEX wcex;
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = WndProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = hInst;
		wcex.hIcon = LoadIcon(hInst, IDI_APPLICATION);
		wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wcex.lpszMenuName = nullptr;
		wcex.lpszClassName = className;
		wcex.hIconSm = LoadIcon(hInst, IDI_APPLICATION);
		ASSERT(0 != RegisterClassEx(&wcex), "Unable to register a window");

		// Create window
		RECT rc = { 0, 0, (LONG)Graphics::g_DisplayWidth, (LONG)Graphics::g_DisplayHeight };
		AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

		g_hWnd = CreateWindow(className, className, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
			rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInst, nullptr);

		ASSERT(g_hWnd != 0);

		InitializeApplication(app);

		ShowWindow(g_hWnd, SW_SHOWDEFAULT);

		do
		{
			MSG msg = {};
			while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			if (msg.message == WM_QUIT)
				break;
		} while (UpdateApplication(app));    // Returns false to quit loop

		TerminateApplication(app);
		Graphics::Shutdown();
	}

	//--------------------------------------------------------------------------------------
	// Called every time the application receives a message
	//--------------------------------------------------------------------------------------
	LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		switch (message)
		{
		case WM_SIZE:
			Display::Resize((UINT)(UINT64)lParam & 0xFFFF, (UINT)(UINT64)lParam >> 16);
			break;

		case WM_DESTROY:
			PostQuitMessage(0);
			break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}

		return 0;
	}

#endif
}