#include "stdafx.h"
#include "LearnHelloWindow.h"
#include "Triangle.h"
#include "Texture.h"
#include "ConstBuffer.h"
#include "FrameBuffer.h"

#include "RenderMeshes.h"

_Use_decl_annotations_
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
	FRenderMeshes sample(960, 540, L"Learn Hello Window");
	return Win32Application::Run(&sample, hInstance, nCmdShow);
}