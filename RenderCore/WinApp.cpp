#include "pch.h"
#include "WinApp.h"
#include <windowsx.h>

namespace WinApp
{
	HWND Mainhandle;
};

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

	case WM_LBUTTONDOWN:
		Game->OnMouseButtonDown(WPara, GET_X_LPARAM(LPara), GET_Y_LPARAM(LPara));
		break;

	case WM_LBUTTONUP:
		Game->OnMouseButtonUp(WPara, GET_X_LPARAM(LPara), GET_Y_LPARAM(LPara));
		break;

	//case WM_MOUSEMOVE:
	//	Game->OnMouseMove(WPara, GET_X_LPARAM(LPara), GET_Y_LPARAM(LPara));
	//	break;

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
		CW_USEDEFAULT, ClientWidth, ClientHeight, 0, 0, AppInstan, GameCore);

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
	//init game core
	if (!GameCore)
	{
		GameCore = new FGameCore(ClientWidth, ClientHeight);
		GameCore->Initialize();
	}

	//init game timer
	if (!GameTimer)
	{
		GameTimer = new FGameTimer();
		//prepare timer start
		GameTimer->Reset();
		GameTimer->Start();
	}
}

void FWinApp::DestroyApp()
{
	//destroy game core
	if (GameCore)
	{
		GameCore->ShutDown();
		delete GameCore;
		GameCore = nullptr;
	}

	//destroy game timer
	if (GameTimer)
	{
		delete GameTimer;
		GameTimer = nullptr;
	}

	//destroy render thread manager
	if (RenderThreadManager)
	{
		RenderThreadManager->StopRenderThread();
		RenderThreadManager.reset();
	}
}


static void CreateRenderingItem_RenderThread(FGameCore* InGame)
{
	FRenderThreadManager::CreateRenderingItems(InGame->GetStaticActors());
}

void FWinApp::InitEngine()
{
	//try to start render thread
	if (!RenderThreadManager)
	{
		RenderThreadManager = std::make_shared<FRenderThreadManager>();
		RenderThreadManager->StartRenderThread(ClientWidth, ClientHeight);
		FRenderThreadCommand CreateRenderItemCommand;
		CreateRenderItemCommand.Wrap(CreateRenderingItem_RenderThread, GameCore);
		RenderThreadManager->PushRenderCommand(CreateRenderItemCommand);

		//pass render thread manager to game core
		GameCore->RenderThreadManager_Weak = RenderThreadManager;
	}

}

void FWinApp::Update()
{
	//wait for render thread processed
	RenderThreadManager->WaitForRenderThreadSingal();

	GameTimer->Tick();
	float DeltaTime = GameTimer->GetDeltaTime();
	if (DeltaTime < GameTimer->GetFrameTime())
	{
		int SleepTime = (int)round(GameTimer->GetFrameTime() - DeltaTime);
		Sleep(SleepTime);
	}
	GameCore->Tick(DeltaTime);

	//todo : notify game thread is completed
	RenderThreadManager->NotifyRenderThreadJob();
	
}
