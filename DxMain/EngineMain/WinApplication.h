#pragma once

#include "DXSample.h"

class DXSample;

class FWinApplication
{
public:
	static int Run(DXSample* Sample, HINSTANCE hInstance, int nCmdShow);
	static HWND GetHWnd() { return m_hwnd; };

protected:
	static LRESULT  CALLBACK WindowProc(HWND Wnd, UINT Message, WPARAM wParam, LPARAM lParam);

private:
	static HWND m_hwnd;
};