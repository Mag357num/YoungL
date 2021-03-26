#include "Display.h"
#include "../Buffer/ColorBuffer.h"
#include "RootSignature.h"
#include "PipelineState.h"
#include "../../pch.h"
#include "../../Misc/Utility.h"
#include "../GraphicsCore.h"

#include <dxgi1_4.h>


#define  SWAP_CHAIN_BUFFER_COUNT 2
DXGI_FORMAT SwapChainFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

namespace GameCore
{
	extern HWND g_hWnd;
}

namespace
{
	float FrameTime = 0.0f;
	uint64_t FrameIndex = 0;
	int64_t FrameStartTike = 0;

}

namespace Graphics
{
	void PreparePresentSDR();
	void CompositeOverlays();

	enum EResolution
	{
		K720p,
		K900p,
		K1080p,
		K1440p,
		K1800p,
		K2160p
	};

	enum EEQuality
	{
		EQAA1x1,
		EQAA1x8,
		EQAA1x16
	};

	EResolution DisplayResolution = K720p;

	const uint32_t NumPredefinedResolution = 6;
	const char* ResolutionLabels[] = {"1280x720", "1600x900" , "1920x1080" ,"2560x1440", "3200x1800", "3840x2160" };

	bool g_EnableHDROutput = false;
	uint32_t g_NativeWidth = 0;
	uint32_t g_NativeHeight = 0;
	uint32_t g_DisplayWidth = 1920;
	uint32_t g_DisplayHeight = 1080;
	FColorBuffer g_PreDisplayBuffer;
	FColorBuffer g_DisplayPlane[SWAP_CHAIN_BUFFER_COUNT];
	UINT g_CurrentBuffer = 0;

	IDXGISwapChain1* s_SwapChain1 = nullptr;
	FRootSignature s_PresentRS;
	FGraphicPSO PresentSDRPSO(L"Core: PresentSDR");

	const char* FilterLabels[] = {"Bilinear", "Sharpening", "Bicubic", "Lanczos"};

	void ResolutionToUINT(EResolution Res, uint32_t& Width, uint32_t& Height)
	{
		switch (Res)
		{
		case Graphics::K720p:
			Width = 1280;
			Height = 720;
			break;
		case Graphics::K900p:
			Width = 1600;
			Height = 900;
			break;
		case Graphics::K1080p:
			Width = 19200;
			Height = 1080;
			break;
		case Graphics::K1440p:
			Width = 25600;
			Height = 1440;
			break;
		case Graphics::K1800p:
			Width = 3200;
			Height = 1800;
			break;
		case Graphics::K2160p:
			Width = 3840;
			Height = 2160;
			break;
		default:
			break;
		}
	}

	void SetNativeResolution()
	{
		uint32_t NativeWidth, NativeHeight;
		ResolutionToUINT(DisplayResolution, NativeWidth, NativeHeight);
		if (g_NativeWidth == NativeWidth && g_NativeHeight == NativeHeight)
		{
			return;
		}

		g_NativeWidth = NativeWidth;
		g_NativeHeight = NativeHeight;

		g_CommandManager.IdleGPU();

	}

	void SetDisplayResolution()
	{
#ifdef _GAMING_DESKTOP
		static int SelectedDisplayRes = DisplayResolution;

		if (SelectedDisplayRes == DisplayResolution)
		{
			return;
		}

		SelectedDisplayRes = DisplayResolution;
		ResolutionToUINT((EResolution)SelectedDisplayRes, g_DisplayWidth, g_DisplayHeight);

		g_CommandManager.IdleGPU();
		Display::Resize(g_DisplayWidth, g_DisplayHeight);
		SetWindowPos(GameCore::g_hWnd, 0, 0, 0, g_DisplayWidth, g_DisplayHeight, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

#endif // _GAMING_DESKTOP

	}

	//sampler 
	D3D12_SAMPLER_DESC SamplerLinearClampDesc;
	D3D12_SAMPLER_DESC SamplerPointClampDesc;

}

using namespace Graphics;

void Display::Initialize()
{
	ASSERT(Graphics::s_SwapChain1 == nullptr, "Swap chain has already by initialied");

	Microsoft::WRL::ComPtr<IDXGIFactory4> DXGIFactor;
	ASSERT_SUCCEEDED(CreateDXGIFactory2(0, IID_PPV_ARGS(&DXGIFactor)));

	DXGI_SWAP_CHAIN_DESC1 SwapChainDesc = {};
	SwapChainDesc.Width = g_DisplayWidth;
	SwapChainDesc.Height = g_DisplayHeight;
	SwapChainDesc.BufferCount = SWAP_CHAIN_BUFFER_COUNT;
	SwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	SwapChainDesc.Format = SwapChainFormat;
	SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	SwapChainDesc.SampleDesc.Count = 1;
	SwapChainDesc.SampleDesc.Quality = 0;
	SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	SwapChainDesc.Scaling = DXGI_SCALING_NONE;
	SwapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;


	DXGI_SWAP_CHAIN_FULLSCREEN_DESC SwapChainFDesc;
	SwapChainFDesc.Windowed = TRUE;
	
	DXGIFactor->CreateSwapChainForHwnd(g_CommandManager.GetCommandQueue(),
		GameCore::g_hWnd,
		&SwapChainDesc, &SwapChainFDesc, nullptr, &s_SwapChain1);


	//get swapchain buffer
	for (size_t i = 0; i < SWAP_CHAIN_BUFFER_COUNT; i++)
	{
		ComPtr<ID3D12Resource> TempDisplay;
		s_SwapChain1->GetBuffer(i, IID_PPV_ARGS(&TempDisplay));

		g_DisplayPlane[i].CreateFromSwapChain(L"Primary swap chain buffer", TempDisplay.Detach());
	}

	//initialize sampler
	SamplerLinearClampDesc = D3D12_SAMPLER_DESC();
	SamplerLinearClampDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	SamplerLinearClampDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	SamplerLinearClampDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	SamplerLinearClampDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	SamplerLinearClampDesc.BorderColor[0] = 1.0f;
	SamplerLinearClampDesc.BorderColor[1] = 1.0f;
	SamplerLinearClampDesc.BorderColor[2] = 1.0f;
	SamplerLinearClampDesc.BorderColor[3] = 1.0f;
	SamplerLinearClampDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	SamplerLinearClampDesc.MinLOD = 0.0f;
	SamplerLinearClampDesc.MaxLOD = D3D12_FLOAT32_MAX;
	SamplerLinearClampDesc.MaxAnisotropy = 16;
	SamplerLinearClampDesc.MipLODBias = 0.0f;

	SamplerPointClampDesc.Filter = D3D12_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;

	//initialize root signature
	s_PresentRS.Reset(4, 2);
	s_PresentRS[0].InitAsDescritporRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 2);
	s_PresentRS[1].InitAsConstants(0, 6, D3D12_SHADER_VISIBILITY_ALL);
	s_PresentRS[2].InitAsBufferSRV(2, D3D12_SHADER_VISIBILITY_PIXEL);
	s_PresentRS[3].InitAsDescritporRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0, 2);
	s_PresentRS.InitStaticSampler(0, SamplerLinearClampDesc);
	s_PresentRS.InitStaticSampler(1, SamplerPointClampDesc);
	s_PresentRS.Finalize(L"Present");

	//initialize pso
	PresentSDRPSO.SetRootSignature(s_PresentRS);

}

void Display::ShutDown()
{
	s_SwapChain1->SetFullscreenState(FALSE, nullptr);
	s_SwapChain1->Release();
	for (UINT i = 0; i < SWAP_CHAIN_BUFFER_COUNT; i++)
	{
		g_DisplayPlane[i].Destroy();
	}

}

void Display::Resize(uint32_t Width, uint32_t Height)
{
	g_CommandManager.IdleGPU();

	g_DisplayWidth = Width;
	g_DisplayHeight = Height;

	for (uint32_t i =0; i < SWAP_CHAIN_BUFFER_COUNT; ++i)
	{
		g_DisplayPlane[i].Destroy();
	}

	ASSERT(s_SwapChain1 != nullptr);
	ASSERT_SUCCEEDED(s_SwapChain1->ResizeBuffers(SWAP_CHAIN_BUFFER_COUNT, Width, Height, SwapChainFormat, 0));

	for (uint32_t i = 0; i < SWAP_CHAIN_BUFFER_COUNT; i++)
	{
		ComPtr<ID3D12Resource> TempResource;
		s_SwapChain1->GetBuffer(i, IID_PPV_ARGS(&TempResource));
		g_DisplayPlane->CreateFromSwapChain(L"Primary SwapChain Buffer", TempResource.Detach());
	}

	g_CurrentBuffer = 0;
	g_CommandManager.IdleGPU();
}

void Display::Present()
{
	
}

uint64_t Graphics::GetFrameCount()
{
	return FrameIndex;
}

float Graphics::GetFrameTime()
{
	return FrameTime;
}

float Graphics::GetFrameRate()
{
	return FrameTime == 0.0f ? 0.0f : 1.0f/ FrameTime;
}


void Graphics::CompositeOverlays()
{

}

void Graphics::PreparePresentSDR()
{

}