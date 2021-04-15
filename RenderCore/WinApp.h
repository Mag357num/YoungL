#pragma once

#include <string>
#include <wrl.h>

#include "GameCore.h"
#include "GameTimer.h"
#include "RenderThreadManager.h"

using namespace Microsoft::WRL;

#define WindowClass L"MainWnd"
#define  MenuName L"D3DEx"
#define WindowTitle L"D3D12Example"


class FWinApp
{
public:
	FWinApp(HINSTANCE Instance) :AppInstan(Instance) {
		ClientWidth = 800;
		ClientHeight = 600;
	}

	~FWinApp(){
	}

	bool InitializeWindow();

	int Run();

	//game update
	void Update();

	//sync change to render thread
	//void UpdateSceneConstantBuffer(FMatrix InView, FMatrix InProj, FVector4D InCamerLoc);

private:
	void InitGame();
	void InitEngine();

	//release game and engine ptr
	void DestroyApp();


	HINSTANCE AppInstan = nullptr;
	//HWND Mainhandle;

	//for graphics
	int ClientWidth = 800;
	int ClientHeight = 600;

	//game and render
	FGameCore* GameCore;
	FGameTimer* GameTimer;

	std::shared_ptr<FRenderThreadManager> RenderThreadManager;
};

