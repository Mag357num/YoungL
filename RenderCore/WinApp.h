#pragma once

#include <string>
#include <wrl.h>

#include "GameCore.h"
#include "Renderer.h"

using namespace Microsoft::WRL;

#define  AssetPath L"Models/ModelSave.Bin"
#define  ShaderPathVS L"Shaders\\TestVS.hlsl"
#define  ShaderPathPS L"Shaders\\TestPS.hlsl"
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

	~FWinApp(){}

	bool InitializeWindow();

	int Run();

	void Update();
	void Render();

private:
	void InitGame();
	void InitEngine();
	void DestroyApp();


	HINSTANCE AppInstan = nullptr;
	//HWND Mainhandle;

	//for graphics
	int ClientWidth = 800;
	int ClientHeight = 600;

	//game and render
	FGameCore* GameCore;
	FRenderer* Renderer;
};

