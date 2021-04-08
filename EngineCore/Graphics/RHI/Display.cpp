#include "pch.h"
#include "Display.h"
#include "../Buffer/ColorBuffer.h"
#include "RootSignature.h"
#include "PipelineState.h"
#include "../../Misc/Utility.h"
#include "../GraphicsCore.h"
#include "SamplerDesc.h"

#include <dxgi1_4.h>


#include "CompiledShaders/ScreenQuadVS.h"
#include "CompiledShaders/PresentSDRPS.h"

#define  SWAP_CHAIN_BUFFER_COUNT 2
DXGI_FORMAT SwapChainFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;

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
	uint32_t g_DisplayWidth = 1280;
	uint32_t g_DisplayHeight = 720;
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
	FSamplerDesc SamplerLinearClampDesc;
	FSamplerDesc SamplerPointClampDesc;

	D3D12_CPU_DESCRIPTOR_HANDLE SamplerLinearClamp;
	D3D12_CPU_DESCRIPTOR_HANDLE SamplerPointClamp;

	D3D12_RASTERIZER_DESC RasterizerTwoSided;
	D3D12_RASTERIZER_DESC RasterizerDefault;

	D3D12_BLEND_DESC PresentSDRBlend;

	D3D12_DEPTH_STENCIL_DESC DepthStateDisabled;

	void InitCommonState()
	{
		//initialize sampler
		SamplerLinearClampDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		SamplerLinearClampDesc.SetTextureAddressMode(D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
		SamplerLinearClamp = SamplerLinearClampDesc.CreateDescriptor();

		SamplerPointClampDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
		SamplerPointClampDesc.SetTextureAddressMode(D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
		SamplerPointClamp = SamplerPointClampDesc.CreateDescriptor();

		//rasterize 
		// Default rasterizer states
		RasterizerDefault.FillMode = D3D12_FILL_MODE_SOLID;
		RasterizerDefault.CullMode = D3D12_CULL_MODE_BACK;
		RasterizerDefault.FrontCounterClockwise = TRUE;
		RasterizerDefault.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
		RasterizerDefault.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
		RasterizerDefault.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
		RasterizerDefault.DepthClipEnable = TRUE;
		RasterizerDefault.MultisampleEnable = FALSE;
		RasterizerDefault.AntialiasedLineEnable = FALSE;
		RasterizerDefault.ForcedSampleCount = 0;
		RasterizerDefault.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

		RasterizerTwoSided = RasterizerDefault;
		RasterizerTwoSided.CullMode = D3D12_CULL_MODE_NONE;

		//blend
		D3D12_BLEND_DESC alphaBlend = {};
		alphaBlend.IndependentBlendEnable = FALSE;
		alphaBlend.RenderTarget[0].BlendEnable = FALSE;
		alphaBlend.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
		alphaBlend.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
		alphaBlend.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		alphaBlend.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
		alphaBlend.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
		alphaBlend.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
		alphaBlend.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
		alphaBlend.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
		alphaBlend.RenderTarget[0].LogicOpEnable = FALSE;
		PresentSDRBlend = alphaBlend;

		//depth stencil
		DepthStateDisabled.DepthEnable = FALSE;
		DepthStateDisabled.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
		DepthStateDisabled.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
		DepthStateDisabled.StencilEnable = FALSE;
		DepthStateDisabled.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
		DepthStateDisabled.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
		DepthStateDisabled.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
		DepthStateDisabled.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
		DepthStateDisabled.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
		DepthStateDisabled.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
		DepthStateDisabled.BackFace = DepthStateDisabled.FrontFace;
	}


	//scene color
	extern FColorBuffer g_SceneColorBuffer;
	//scene depth
	extern FDepthBuffer g_SceneDepthBuffer;

	extern Microsoft::WRL::ComPtr<IDXGIFactory4> g_DxgiFactory;;

}

using namespace Graphics;

void Display::Initialize()
{
	ASSERT(Graphics::s_SwapChain1 == nullptr, "Swap chain has already be initialied");

	//g_DisplayWidth = 1280;
	//g_DisplayHeight = 720;

	DXGI_SWAP_CHAIN_DESC1 SwapChainDesc = {};
	SwapChainDesc.Width = g_DisplayWidth;
	SwapChainDesc.Height = g_DisplayHeight;
	SwapChainDesc.Format = SwapChainFormat;
	SwapChainDesc.BufferCount = SWAP_CHAIN_BUFFER_COUNT;
	SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	SwapChainDesc.SampleDesc.Count = 1;
	SwapChainDesc.SampleDesc.Quality = 0;
	SwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
	SwapChainDesc.Scaling = DXGI_SCALING_NONE;


	//DXGI_SWAP_CHAIN_FULLSCREEN_DESC SwapChainFDesc;
	//SwapChainFDesc.Windowed = TRUE;
	
	g_DxgiFactory->CreateSwapChainForHwnd(g_CommandManager.GetCommandQueue(),
		GameCore::g_hWnd,
		&SwapChainDesc, nullptr, nullptr, &s_SwapChain1);


	//get swapchain buffer
	for (size_t i = 0; i < SWAP_CHAIN_BUFFER_COUNT; i++)
	{
		ComPtr<ID3D12Resource> TempDisplay;
		s_SwapChain1->GetBuffer(i, IID_PPV_ARGS(&TempDisplay));
		g_DisplayPlane[i].CreateFromSwapChain(L"Primary swap chain buffer", TempDisplay.Detach());
	}

	Graphics::InitCommonState();

	//initialize root signature
	s_PresentRS.Reset(1, 1);
	s_PresentRS[0].InitAsDescritporRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 1);
	//s_PresentRS[1].InitAsConstants(0, 6, D3D12_SHADER_VISIBILITY_ALL);
	//s_PresentRS[2].InitAsBufferSRV(2, D3D12_SHADER_VISIBILITY_PIXEL);
	//s_PresentRS[3].InitAsDescritporRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0, 1);
	s_PresentRS.InitStaticSampler(0, SamplerLinearClampDesc);
	//s_PresentRS.InitStaticSampler(1, SamplerPointClampDesc);
	s_PresentRS.Finalize(L"Present");

	//initialize pso
	PresentSDRPSO.SetRootSignature(s_PresentRS);
	PresentSDRPSO.SetBlendState(Graphics::PresentSDRBlend);
	//PresentSDRPSO.SetBlendState(CD3DX12_BLEND_DESC(D3D12_DEFAULT));
	//PresentSDRPSO.SetRasterizerState(Graphics::RasterizerTwoSided);
	D3D12_RASTERIZER_DESC Rasterizer = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	Rasterizer.CullMode = D3D12_CULL_MODE_FRONT;
	PresentSDRPSO.SetRasterizerState(Rasterizer);

	PresentSDRPSO.SetDepthStencilState(Graphics::DepthStateDisabled);
	PresentSDRPSO.SetSampleMask(0xFFFFFFFF);
	PresentSDRPSO.SetInputLayout(0, nullptr);
	PresentSDRPSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
	PresentSDRPSO.SetVertexShader(g_ScreenQuadVS, sizeof(g_ScreenQuadVS));
	PresentSDRPSO.SetPixelShader(g_PresentSDRPS, sizeof(g_PresentSDRPS));
	PresentSDRPSO.SetRenderTargetFormat(SwapChainFormat, DXGI_FORMAT_UNKNOWN);
	PresentSDRPSO.Finalize();

	SetNativeResolution();

	//initialize scene color && scene depth
	Graphics::g_SceneColorBuffer.Create(L"Main Scene Color", g_DisplayWidth, g_DisplayHeight, 1, SwapChainFormat);
	g_PreDisplayBuffer.Create(L"PreDisplay Buffer", g_DisplayWidth, g_DisplayHeight, 1, SwapChainFormat);

}

void Display::ShutDown()
{
	s_SwapChain1->SetFullscreenState(FALSE, nullptr);
	s_SwapChain1->Release();
	for (UINT i = 0; i < SWAP_CHAIN_BUFFER_COUNT; i++)
	{
		g_DisplayPlane[i].Destroy();
	}

	g_SceneColorBuffer.Destroy();

}

void Display::Resize(uint32_t Width, uint32_t Height)
{
	g_CommandManager.IdleGPU();

	g_DisplayWidth = Width;
	g_DisplayHeight = Height;

	g_PreDisplayBuffer.Create(L"PreDisplay Buffer", Width, Height, 1, SwapChainFormat);

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
		g_DisplayPlane[i].CreateFromSwapChain(L"Primary SwapChain Buffer", TempResource.Detach());
	}

	g_CurrentBuffer = 0;
	g_CommandManager.IdleGPU();
}

void Display::Present()
{
	PreparePresentSDR();

	g_CurrentBuffer = (g_CurrentBuffer + 1) % 2;

	UINT PresentInterval = 0;// if open vsync, vsync 
	s_SwapChain1->Present(PresentInterval, 0);

	SetNativeResolution();
	SetDisplayResolution();
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
	FGraphicsContext& PresentContext = FGraphicsContext::Begin(L"Present");
	PresentContext.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	PresentContext.SetViewportAndScissor(0, 0, g_NativeWidth, g_NativeHeight);
	PresentContext.ClearColor(g_DisplayPlane[g_CurrentBuffer]);
	PresentContext.TransitionResource(g_DisplayPlane[g_CurrentBuffer], D3D12_RESOURCE_STATE_RENDER_TARGET);
	PresentContext.SetRenderTargets(g_DisplayPlane[g_CurrentBuffer].GetRtv());

	PresentContext.SetRootSignature(s_PresentRS);
	PresentContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	PresentContext.SetDynamicDescriptor(0, 0, g_SceneColorBuffer.GetSrv());
	//FColorBuffer& Dest = g_DisplayPlane[g_CurrentBuffer];
	PresentContext.SetPipelineState(PresentSDRPSO);
	PresentContext.Draw(3);

	PresentContext.TransitionResource(g_DisplayPlane[g_CurrentBuffer], D3D12_RESOURCE_STATE_PRESENT);

	PresentContext.Finish();
}