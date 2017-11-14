#include "stdafx.h"
#include "LearnHelloWindow.h"

_Use_decl_annotations_
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
	LearnHelloWindow sample(1280, 720, L"Learn Hello Window");
	return Win32Application::Run(&sample, hInstance, nCmdShow);
}