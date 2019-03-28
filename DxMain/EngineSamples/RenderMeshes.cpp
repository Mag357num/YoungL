#include "stdafx.h"
#include "RenderMeshes.h"
#include "Win32Application.h"

FRenderMeshes::FRenderMeshes(UINT Width, UINT Height, std::wstring name)
	:DXSample(Width, Height, name)
{
}

void FRenderMeshes::OnInit()
{
	LoadPipeline();
	LoadAsset();
}

void FRenderMeshes::LoadPipeline()
{
	UINT DxGiFactoryFlag = 0;

#if (_DEBUG)

	ComPtr<ID3D12Debug> DebugController;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&DebugController))))
	{
		DebugController->EnableDebugLayer();

		DxGiFactoryFlag |= DXGI_CREATE_FACTORY_DEBUG;
	}
#endif

	ThrowIfFailed(CreateDXGIFactory2(DxGiFactoryFlag, IID_PPV_ARGS(&DXFactory)));
	
	ComPtr<IDXGIAdapter1> TempAdapter;
	for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != DXFactory->EnumAdapters1(adapterIndex, &TempAdapter); adapterIndex++)
	{
		DXGI_ADAPTER_DESC1 AdapterDesc;
		TempAdapter->GetDesc1(&AdapterDesc);
		if (AdapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			continue;
		}

		if (SUCCEEDED(D3D12CreateDevice(TempAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_Device))))
		{
			break;
		}
	}

	D3D12_COMMAND_QUEUE_DESC CommandQueueDesc = {};
	CommandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	CommandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	m_Device->CreateCommandQueue(&CommandQueueDesc, IID_PPV_ARGS(&m_CommandQueue));
	
	DXGI_SWAP_CHAIN_DESC1 SwapChainDesc = {};
	SwapChainDesc.BufferCount = FrameCount;
	SwapChainDesc.Width = m_width;
	SwapChainDesc.Height = m_height;
	SwapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	SwapChainDesc.SampleDesc.Count = 1;

	ComPtr<IDXGISwapChain1> SwapChain;
	DXFactory->CreateSwapChainForHwnd(
		m_CommandQueue.Get(),
		Win32Application::GetHWND(),
		&SwapChainDesc,
		nullptr,
		nullptr,
		&SwapChain);

	DXFactory->MakeWindowAssociation(Win32Application::GetHWND(), DXGI_MWA_NO_ALT_ENTER);
	
	ThrowIfFailed(SwapChain.As(&m_SwapChain));

	m_FrameIndex = m_SwapChain->GetCurrentBackBufferIndex();

	D3D12_DESCRIPTOR_HEAP_DESC HeapDesc = {};
	HeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	HeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	HeapDesc.NumDescriptors = FrameCount;

	ThrowIfFailed(m_Device->CreateDescriptorHeap(&HeapDesc, IID_PPV_ARGS(&m_DescriptorHeap)));
	m_RtvDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtv_Handle(m_DescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	for (UINT n = 0; n < FrameCount; n++)
	{
		ThrowIfFailed(m_SwapChain->GetBuffer(n, IID_PPV_ARGS(&m_RenderTarget[n])));
		m_Device->CreateRenderTargetView(m_RenderTarget[n].Get(), nullptr, rtv_Handle);
		rtv_Handle.Offset(1, m_RtvDescriptorSize);
	}

	m_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_CommandAllocator));
}

void FRenderMeshes::LoadAsset()
{
	ThrowIfFailed(m_Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_CommandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_CommandList)));
	ThrowIfFailed(m_CommandList->Close());

	ThrowIfFailed(m_Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence)));
	m_FenceValue = 1;

	m_FenceEvent = CreateEvent(nullptr, false, false, nullptr);
	if (m_FenceEvent == nullptr)
	{
		ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
	}
}


void FRenderMeshes::OnUpdate()
{

}

void FRenderMeshes::OnRender()
{
	PopulateCommandList();

	ID3D12CommandList* ppCommandLists[] = { m_CommandList.Get() };
	m_CommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	ThrowIfFailed(m_SwapChain->Present(1, 0));

	WaitForPreviousFrame();
}

void FRenderMeshes::PopulateCommandList()
{
	ThrowIfFailed(m_CommandAllocator->Reset());

	ThrowIfFailed(m_CommandList->Reset(m_CommandAllocator.Get(), nullptr));

	m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_RenderTarget[m_FrameIndex].Get(),D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtv_Handle(m_DescriptorHeap->GetCPUDescriptorHandleForHeapStart(), m_FrameIndex, m_RtvDescriptorSize);
	const float ClearColor[] = {0.0f, 0.2f, 0.4f, 1.0f};
	m_CommandList->ClearRenderTargetView(rtv_Handle, ClearColor, 0, nullptr);

	m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_RenderTarget[m_FrameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	ThrowIfFailed(m_CommandList->Close());
}

void FRenderMeshes::WaitForPreviousFrame()
{
	UINT64 FenceValue = m_FenceValue;

	ThrowIfFailed(m_CommandQueue->Signal(m_Fence.Get(), FenceValue));
	m_FenceValue++;

	if (m_Fence->GetCompletedValue() < FenceValue)
	{
		m_Fence->SetEventOnCompletion(FenceValue, m_FenceEvent);
		ThrowIfFailed(WaitForSingleObject(m_FenceEvent, INFINITE));
	}

	m_FrameIndex = m_SwapChain->GetCurrentBackBufferIndex();
}

void FRenderMeshes::OnDestroy()
{

}