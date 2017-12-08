#include "stdafx.h"
#include "LearnHelloWindow.h"
#include "Triangle.h"
#include "Texture.h"
#include "ConstBuffer.h"

_Use_decl_annotations_
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
	ConstBufferSample sample(1280, 720, L"Learn Hello Window");
	return Win32Application::Run(&sample, hInstance, nCmdShow);
}