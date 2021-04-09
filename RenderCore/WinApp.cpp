#include "WinApp.h"

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
			Render();
		}
	}

	DestroyApp();

	return (int)Msg.wParam;
}

void FWinApp::InitGame()
{
	if (!GameCore)
	{
		GameCore = new FGameCore();
		GameCore->Initialize();
	}
}

void FWinApp::InitEngine()
{
	//init render core
	if (!Renderer)
	{
		Renderer = new FRenderer();
		Renderer->CreateRHIContext(ClientWidth, ClientHeight);
	}

	//we does't have game && render sync mechanism; 
	// pass to render directly
	Renderer->CreateRenderingItem(GameCore->GetGeometries());
}

void FWinApp::DestroyApp()
{
	if (GameCore)
	{
		GameCore->ShutDown();
		delete GameCore;
		GameCore = nullptr;
	}

	if (Renderer)
	{
		Renderer->DestroyRHIContext();
		delete Renderer;
		Renderer = nullptr;
	}
}

void FWinApp::Update()
{
	GameCore->Tick();
}

void FWinApp::Render()
{
	Renderer->RenderObjects();
}

