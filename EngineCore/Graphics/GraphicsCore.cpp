#include "GraphicsCore.h"
#include "RHI/CommandListManager.h"
#include "RHI/CommandSignature.h"
#include "RHI/Display.h"
#include <dxgi1_6.h>

namespace Graphics
{
	//class FCommandListmanager;

	ID3D12Device* g_Device = nullptr;
	FCommandListmanager g_CommandManager;
	FContextManager g_ContextManager;
	FCommandSignature DrawIndirectCommandSignature;

	FDescriptorAllocator g_DescriptorAllocator[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES] =
	{
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
		D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
		D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
		D3D12_DESCRIPTOR_HEAP_TYPE_DSV
	};


}

void Graphics::Initialize(void)
{
	Microsoft::WRL::ComPtr<ID3D12Device> TempDevice;

	//enable debug layer
	uint32_t UseDebugFlag = 0;
#if _DEBUG
	UseDebugFlag = 1;
#endif

	if (UseDebugFlag)
	{
		Microsoft::WRL::ComPtr<ID3D12Debug> DebugController;
		D3D12GetDebugInterface(IID_PPV_ARGS(&DebugController));
		DebugController->EnableDebugLayer();
	}
	
	//create factory
	UINT CreateDxgiFactorFlag = 0;
	Microsoft::WRL::ComPtr<IDXGIFactory6> DxgiFactory;
	ASSERT_SUCCEEDED(CreateDXGIFactory2(CreateDxgiFactorFlag, IID_PPV_ARGS(&DxgiFactory)));

	//create device
	Microsoft::WRL::ComPtr<IDXGIAdapter1> AdapterPtr;
	SIZE_T MaxVedioSize = 0;
	for (uint32_t Idx = 0; DXGI_ERROR_NOT_FOUND != DxgiFactory->EnumAdapters1(Idx, &AdapterPtr); ++Idx)
	{
		DXGI_ADAPTER_DESC1 AdapterDesc;
		AdapterPtr->GetDesc1(&AdapterDesc);
		if (AdapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			continue;
		}


		//get largest vedio memory device
		if (AdapterDesc.DedicatedVideoMemory > MaxVedioSize &&
			SUCCEEDED(D3D12CreateDevice(AdapterPtr.Get(), D3D_FEATURE_LEVEL_11_0, MY_IID_PPV_ARGS(&TempDevice)))
			)
		{
			MaxVedioSize = AdapterDesc.DedicatedVideoMemory;
		}

	}
	
	if (MaxVedioSize > 0)
	{
		g_Device = TempDevice.Detach();
	}

	if (g_Device == nullptr)
	{
		//assert error
		ASSERT(0);
	}
	

	g_CommandManager.Create(g_Device);
	Display::Initialize();
}

void Graphics::Terminate(void)
{

}

void Graphics::Shutdown(void)
{
	g_CommandManager.IdleGPU();

	FCommandContext::DestroyAllContexts();
	g_CommandManager.ShutDown();

	FPSO::DestroyAll();
	FRootSignature::DestroyAll();
	FDescriptorAllocator::DestroyAll();

	Display::ShutDown();

	if (g_Device)
	{
		g_Device->Release();
		g_Device = nullptr;
	}
}
