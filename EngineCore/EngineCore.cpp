// EngineCore.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
// Windows 头文件
#include <windows.h>
// C 运行时头文件
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <iostream>

#include "pch.h"

#include "GameCore.h"

using namespace GameCore;

class FRenderTest : public GameCore::IGameApp
{
public:
	FRenderTest(void) {}

	virtual void Startup(void) override;
	virtual void Cleanup(void) override;

	virtual void Update(float deltaT) override;
	virtual void RenderScene(void) override;

	//virtual bool IsDone() override;
	virtual void RenderUI(class GraphicsContext&) override;

private:

};

CREATE_APPLICATION(FRenderTest)

void FRenderTest::Startup()
{

}

void FRenderTest::Cleanup()
{

}

void FRenderTest::Update(float deltaT)
{

}

void FRenderTest::RenderScene()
{

}

//bool FRenderTest::IsDone()
//{
//	return IGameApp::IsDone();
//}

void FRenderTest::RenderUI(class GraphicsContext&)
{

}

//int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
//	_In_opt_ HINSTANCE hPrevInstance,
//	_In_ LPWSTR    lpCmdLine,
//	_In_ int       nCmdShow)
//{
//	return 0;
//}

