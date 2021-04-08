#include "RHIContext_D3D12.h"

#include "../../d3dx12.h"

namespace WinApp
{
	extern HWND Mainhandle;
};

void FRHIContext_D3D12::InitializeRHI(int InWidth, int InHeight)
{
	ClientWidth = InWidth;
	ClientHeight = InHeight;

	//debug controller
#if defined(DBUG) || defined(_DEBUG)
	Microsoft::WRL::ComPtr<ID3D12Debug> DebugInterface;
	D3D12GetDebugInterface(IID_PPV_ARGS(&DebugInterface));
	DebugInterface->EnableDebugLayer();
#endif


	//create factory
	CreateDXGIFactory1(IID_PPV_ARGS(&M_Factory));

	//enumerate adapter device
	//try to create hard ware device
	HRESULT Result = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&M_Device));

	if (FAILED(Result))
	{
		Microsoft::WRL::ComPtr<IDXGIAdapter> WarpAdapter;
		M_Factory->EnumWarpAdapter(IID_PPV_ARGS(&WarpAdapter));
		D3D12CreateDevice(WarpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&M_Device));
	}

	M_RtvDescriptorSize = M_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	M_DsvDescriptorSize = M_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	M_CbvSrvUavDescriptorSize = M_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	//create commang objects
	M_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&M_CommandAllocator));
	D3D12_COMMAND_QUEUE_DESC QueueDesc = {};
	QueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	QueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	M_Device->CreateCommandQueue(&QueueDesc, IID_PPV_ARGS(&M_CommandQueue));
	M_Device->CreateCommandList(1, D3D12_COMMAND_LIST_TYPE_DIRECT, M_CommandAllocator.Get(), nullptr, IID_PPV_ARGS(&M_CommandList));

	M_CommandList->Close();

	//create fence
	M_Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&M_Fence));

	//create swapchain
	M_SwapChain.Reset();
	DXGI_SWAP_CHAIN_DESC SwapChainDesc;
	SwapChainDesc.BufferCount = M_SwapchainBackbufferCount;
	SwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	SwapChainDesc.SampleDesc.Count = 1;
	SwapChainDesc.SampleDesc.Quality = 0;
	SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	SwapChainDesc.OutputWindow = WinApp::Mainhandle;
	SwapChainDesc.Windowed = true;
	SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	SwapChainDesc.BufferDesc.Width = ClientWidth;
	SwapChainDesc.BufferDesc.Height = ClientHeight;
	SwapChainDesc.BufferDesc.Format = M_BackBufferFormat;
	SwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	SwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	SwapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_CENTERED;
	SwapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;


	M_Factory->CreateSwapChain(M_CommandQueue.Get(), &SwapChainDesc, M_SwapChain.GetAddressOf());

	//create discriptor heap
	D3D12_DESCRIPTOR_HEAP_DESC RtvHeapDesc;
	RtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	RtvHeapDesc.NumDescriptors = M_SwapchainBackbufferCount;
	RtvHeapDesc.NodeMask = 0;
	RtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	M_Device->CreateDescriptorHeap(&RtvHeapDesc, IID_PPV_ARGS(&M_RtvHeap));

	D3D12_DESCRIPTOR_HEAP_DESC DsvHeapDesc;
	DsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	DsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	DsvHeapDesc.NodeMask = 0;
	DsvHeapDesc.NumDescriptors = 1;

	M_Device->CreateDescriptorHeap(&DsvHeapDesc, IID_PPV_ARGS(&M_DsvHeap));

	//initialize back buffer
	OnResize();
}

void FRHIContext_D3D12::Resize(int InWidth, int InHeight)
{
	ClientWidth = InWidth;
	ClientHeight = InHeight;
	OnResize();
}

void FRHIContext_D3D12::OnResize()
{
	FlushCommandQueue();

	//reset command list
	M_CommandList->Reset(M_CommandAllocator.Get(), nullptr);

	//reset back buffer;
	for (int i = 0; i < M_SwapchainBackbufferCount; ++i)
	{
		M_BackBuffer[i].Reset();
	}
	M_DepthStencilBuffer.Reset();

	//resize swapchain
	M_SwapChain->ResizeBuffers(M_SwapchainBackbufferCount, ClientWidth, ClientHeight,
		M_BackBufferFormat, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);

	M_CurrentBackBuffer = 0;

	//get back buffer from swap chain && create rtv
	CD3DX12_CPU_DESCRIPTOR_HANDLE Rtvhandle(M_RtvHeap->GetCPUDescriptorHandleForHeapStart());
	for (int i = 0; i < M_SwapchainBackbufferCount; ++i)
	{
		M_SwapChain->GetBuffer(i, IID_PPV_ARGS(&M_BackBuffer[i]));
		M_Device->CreateRenderTargetView(M_BackBuffer[i].Get(), nullptr, Rtvhandle);
		Rtvhandle.Offset(1, M_RtvDescriptorSize);
	}

	//create depth staencil buffer and dsv
	D3D12_RESOURCE_DESC DRDesc;
	DRDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	DRDesc.Alignment = 0;
	DRDesc.Width = ClientWidth;
	DRDesc.Height = ClientHeight;
	DRDesc.DepthOrArraySize = 1;
	DRDesc.MipLevels = 1;

	// Correction 11/12/2016: SSAO chapter requires an SRV to the depth buffer to read from 
	// the depth buffer.  Therefore, because we need to create two views to the same resource:
	//   1. SRV format: DXGI_FORMAT_R24_UNORM_X8_TYPELESS
	//   2. DSV Format: DXGI_FORMAT_D24_UNORM_S8_UINT
	// we need to create the depth buffer resource with a typeless format.  
	DRDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;

	DRDesc.SampleDesc.Count = 1;
	DRDesc.SampleDesc.Quality = 0;
	DRDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	DRDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;


	D3D12_CLEAR_VALUE ClearValue;
	ClearValue.Format = M_DepthStencilFormat;
	ClearValue.DepthStencil.Depth = 1.0f;
	ClearValue.DepthStencil.Stencil = 0;
	CD3DX12_HEAP_PROPERTIES HeapProperty(D3D12_HEAP_TYPE_DEFAULT);
	M_Device->CreateCommittedResource(&HeapProperty, D3D12_HEAP_FLAG_NONE, &DRDesc, D3D12_RESOURCE_STATE_COMMON,
		&ClearValue, IID_PPV_ARGS(&M_DepthStencilBuffer));

	CD3DX12_CPU_DESCRIPTOR_HANDLE DsvHandle(M_DsvHeap->GetCPUDescriptorHandleForHeapStart());
	D3D12_DEPTH_STENCIL_VIEW_DESC DsvDesc;
	DsvDesc.Format = M_DepthStencilFormat;
	DsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	DsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	DsvDesc.Texture2D.MipSlice = 0;
	M_Device->CreateDepthStencilView(M_DepthStencilBuffer.Get(), &DsvDesc, DsvHandle);

	//transilate dsv stato to depth
	CD3DX12_RESOURCE_BARRIER Barrier = CD3DX12_RESOURCE_BARRIER::Transition(M_DepthStencilBuffer.Get(), D3D12_RESOURCE_STATE_COMMON,
		D3D12_RESOURCE_STATE_DEPTH_WRITE);
	M_CommandList->ResourceBarrier(1, &Barrier);

	//excute command list
	M_CommandList->Close();
	ID3D12CommandList* cmdLists[] = { M_CommandList.Get() };
	M_CommandQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);

	//flush command queue
	FlushCommandQueue();

	//update screen view port
	M_ScreenViewport.Width = static_cast<float>(ClientWidth);
	M_ScreenViewport.Height = static_cast<float>(ClientHeight);
	M_ScreenViewport.TopLeftX = 0;
	M_ScreenViewport.TopLeftY = 0;
	M_ScreenViewport.MaxDepth = 1.0f;
	M_ScreenViewport.MinDepth = 0.0f;

	M_ScissorRect = { 0,0, ClientWidth, ClientHeight };
}


void FRHIContext_D3D12::FlushCommandQueue()
{
	M_CurrentFenceValue++;

	M_CommandQueue->Signal(M_Fence.Get(), M_CurrentFenceValue);

	if (M_Fence->GetCompletedValue() < M_CurrentFenceValue)
	{
		HANDLE EventHandle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);
		M_Fence->SetEventOnCompletion(M_CurrentFenceValue, EventHandle);

		WaitForSingleObject(EventHandle, INFINITE);
		CloseHandle(EventHandle);
	}
}