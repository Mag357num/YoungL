#include "stdsfx.h"
#include "Sample.h"

HWND m_hwnd = nullptr;

//main message handler for the sample
LRESULT CALLBACK WinProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	return DefWindowProc(hwnd, message, lparam, wparam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
	//sample
	Sample pSample;

	//pass the command line
	int argc;
	LPWSTR* argv = CommandLineToArgvW(GetCommandLine(), &argc);

	//Dx sample parse command line
	LocalFree(argv);


	//init window
	WNDCLASSEX windowClass = { 0 };

	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_VREDRAW | CS_HREDRAW;
	windowClass.lpfnWndProc = WinProc;
	windowClass.hInstance = hInstance;
	windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
	windowClass.lpszClassName = L"SampleClass";

	//register class
	RegisterClassEx(&windowClass);

	//window rect
	RECT windowRect = { 0, 0, 540, 360};
	AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, false);

	//create window
	m_hwnd = CreateWindow(
		windowClass.lpszClassName,
		L"sdfsfsdfsdf",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		nullptr,
		nullptr,
		hInstance,
		&pSample
	);


	//initialize dx sample

	//show window
	ShowWindow(m_hwnd, nCmdShow);

	//msg
	MSG msg = {};

	while (msg.message != WM_QUIT)
	{
		//process any mesages in the queue
		if(PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			DispatchMessage(&msg);
		}
	}
	
	return static_cast<char>(msg.wParam);
}

