#pragma once

#include "resource.h"
#include "D3DExample.h"


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	DXExample Example(hInstance);
	if (Example.Initialize())
	{
		return Example.Run();
	}
	else
	{
		return 0;
	}
}