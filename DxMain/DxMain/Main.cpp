

HWND m_hwnd = nullptr;

//using namespace WRL.ComPtr;

//main message handler for the sample
LRESULT CALLBACK WinProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	switch (message)
	{
	//case WM_CREATE:
	//{
	//	// Save the DXSample* passed in to CreateWindow.
	//	LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
	//	SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
	//}
	return 0;

	case WM_KEYDOWN:
		//if (pSample)
		//{
		//	pSample->OnKeyDown(static_cast<UINT8>(wParam));
		//}
		return 0;

	case WM_KEYUP:
		//if (pSample)
		//{
		//	pSample->OnKeyUp(static_cast<UINT8>(wParam));
		//}
		return 0;

	case WM_PAINT:
		//if (pSample)
		//{
		//	pSample->OnUpdate();
		//	pSample->OnRender();
		//}
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	// Handle any messages the switch statement didn't.
	return DefWindowProc(hwnd, message, wparam, lparam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
	//sample
	Sample* pSample = new Sample();

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
	//load pipeline
	pSample->LoadPipeline();


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

