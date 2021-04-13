// RenderCore.cpp : 定义应用程序的入口点。
//

#pragma once

#include "resource.h"
#include <windows.h>
#include "WinApp.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{

	FWinApp WinApp(hInstance);
	if (WinApp.InitializeWindow())
	{
		return WinApp.Run();
	}
	else
	{
		return 0;
	}
}