// RenderCore.cpp : 定义应用程序的入口点。
//

//#include "RenderCore.h"

#pragma once

#include "resource.h"
//#include "D3DExample.h"
#include "WinApp.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	//DXExample Example(hInstance);
	//if (Example.Initialize())
	//{
	//	return Example.Run();
	//}
	//else
	//{
	//	return 0;
	//}

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