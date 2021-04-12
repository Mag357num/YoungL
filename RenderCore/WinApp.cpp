#include "WinApp.h"

namespace WinApp
{
	HWND Mainhandle;
};

using namespace RenderFrameSync;

LRESULT CALLBACK WndProc_CallBack(HWND Hwnd, UINT Msg, WPARAM WPara, LPARAM LPara)
{
	FGameCore* Game = reinterpret_cast<FGameCore*>(GetWindowLongPtr(Hwnd, GWLP_USERDATA));
	switch (Msg)
	{
	case WM_CREATE:
	{
		// Save the DXSample* passed in to CreateWindow.
		LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(LPara);
		SetWindowLongPtr(Hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
	}
	return 0;

	case WM_KEYDOWN:
		if (Game)
		{
			Game->OnKeyDown(static_cast<UINT8>(WPara));
		}
		return 0;

	case WM_KEYUP:
		if (Game)
		{
			Game->OnKeyUp(static_cast<UINT8>(WPara));
		}
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	// Handle any messages the switch statement didn't.
	return DefWindowProc(Hwnd, Msg, WPara, LPara);
}


bool FWinApp::InitializeWindow()
{
	//window class
	WNDCLASS WDClass;
	WDClass.hInstance = AppInstan;
	WDClass.lpfnWndProc = WndProc_CallBack;
	WDClass.cbClsExtra = 0;
	WDClass.cbWndExtra = 0;
	WDClass.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	WDClass.hCursor = LoadCursor(0, IDC_ARROW);
	WDClass.hIcon = LoadIcon(0, IDI_APPLICATION);
	WDClass.style = CS_HREDRAW | CS_VREDRAW;
	WDClass.lpszClassName = WindowClass;
	WDClass.lpszMenuName = MenuName;

	RegisterClass(&WDClass);
	//adjust, create window
	RECT Rec = { 0, 0, ClientWidth, ClientHeight };
	AdjustWindowRect(&Rec, WS_OVERLAPPEDWINDOW, false);
	//int Width = Rec.right - Rec.left;
	//int Height = Rec.bottom - Rec.top;

	InitGame();

	WinApp::Mainhandle = CreateWindow(WindowClass, WindowTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 
		CW_USEDEFAULT, ClientWidth, ClientHeight, 0, 0, AppInstan, GameCore.get());

	//show window
	ShowWindow(WinApp::Mainhandle, SW_SHOW);
	UpdateWindow(WinApp::Mainhandle);

	InitEngine();

	return true;
}

int FWinApp::Run()
{
	MSG Msg = { 0 };

	while (Msg.message != WM_QUIT)
	{
		if (PeekMessage(&Msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&Msg);
			DispatchMessage(&Msg);
		}
		else
		{
			Update();
		}
	}

	DestroyApp();

	return (int)Msg.wParam;
}

void FWinApp::InitGame()
{
	if (!GameCore)
	{
		GameCore = std::make_shared<FGameCore>();
		GameCore->Initialize();
	}
}


static void CreateRenderingItem_RenderThread(FRenderer* InRender, FGameCore* InGame)
{
	InRender->CreateRenderingItem(InGame->GetGeometries());
}

void FWinApp::InitEngine()
{
	//try to start render thread
	if (!RenderThread)
	{
		RenderThread = new FRenderThread();
		RenderThread->StartThread(ClientWidth, ClientHeight, GameCore);

		std::function<void(FRenderer*, FGameCore*)> Task;
		Task = CreateRenderingItem_RenderThread;
		RenderThread->PushTask(Task);
	}

}

void FWinApp::DestroyApp()
{
	if (GameCore)
	{
		GameCore->ShutDown();
		GameCore.reset();
	}


	if (RenderThread)
	{
		RenderThread->StopThread();
		delete RenderThread;
		RenderThread = nullptr;
	}
}

void FWinApp::Update()
{
	while (RenderFrameSync::FrameSyncFence >= 1)
	{
		Sleep(10);
	}

	GameCore->Tick();

	RenderFrameSync::FrameSyncFence++;
}
