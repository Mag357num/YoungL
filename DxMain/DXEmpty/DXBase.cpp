#include "DXBase.h"

DXBase::DXBase()
{

}

DXBase::DXBase(UINT inWidth, UINT inHeight, std::wstring InName)
{
	width = inWidth;
	height = inHeight;
	WindowName = InName;
}

DXBase::~DXBase()
{
}

UINT DXBase::getWidth()
{
	return width;
}

UINT DXBase::getHeight()
{
	return height;
}

std::wstring DXBase::getName()
{
	return WindowName;
}

void DXBase::setWND(HWND* inWND)
{
	savedWND = *inWND;
}

void DXBase::initPipeline()
{
	LoadPipeLine();
	LoadAssets();
}

void DXBase::onUpdate()
{

}

void DXBase::onRender()
{
	// Record all the commands we need to render the scene into the command list.
	populateCommandList();

	ID3D12CommandList* ppCommandList[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(ppCommandList), ppCommandList);

	ThrowIfFailed(m_swapChain->Present(1, 0));
	waitForPreviousFrame();
}

void DXBase::onDestroy()
{
	// Ensure that the GPU is no longer referencing resources that are about to be
	// cleaned up by the destructor.
	waitForPreviousFrame();

	CloseHandle(m_fenceEvent);
}

//get hardware adapter
_Use_decl_annotations_
void DXBase::GetHardWareAdapter(IDXGIFactory4* pFactory, IDXGIAdapter1** ppAdapter)
{
	ComPtr<IDXGIAdapter1> adapter;
	*ppAdapter = nullptr;

	for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != pFactory->EnumAdapters1(adapterIndex, &adapter); ++adapterIndex)
	{
		DXGI_ADAPTER_DESC1 desc;
		adapter->GetDesc1(&desc);

		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			continue;
		}

		if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
		{
			break;
		}
	}

	*ppAdapter = adapter.Detach();
}

void DXBase::LoadPipeLine()
{
	UINT dxgiFactoryFlags = 0;

#if defined(_DEBUG)
	// Enable the debug layer (requires the Graphics Tools "optional feature").
	// NOTE: Enabling the debug layer after device creation will invalidate the active device.
	{
		ComPtr<ID3D12Debug> debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();

			// Enable additional debug layers.
			dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
		}
	}
#endif

	ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)));

	//get harware adapter
	GetHardWareAdapter(factory.Get(), &hardwareAdapter);

	//create device
	ThrowIfFailed(D3D12CreateDevice(hardwareAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_Device)));

	//CREATE COMMAND QUEUE
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	ThrowIfFailed(m_Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));

	DXGI_SWAP_CHAIN_DESC1 swapDesc = {};
	swapDesc.BufferCount = FrameCount;
	swapDesc.Width = 540;
	swapDesc.Height = 360;
	swapDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	ComPtr<IDXGISwapChain1> swapChain;
	ThrowIfFailed(
		factory->CreateSwapChainForHwnd(
			m_commandQueue.Get(),
			savedWND,
			&swapDesc,
			nullptr,
			nullptr,
			&swapChain
		)
	);

	//make association
	ThrowIfFailed(factory->MakeWindowAssociation(savedWND, DXGI_MWA_NO_ALT_ENTER));

	//set to m_swapchain
	ThrowIfFailed(swapChain.As(&m_swapChain));

	UINT m_FrameIndex = m_swapChain->GetCurrentBackBufferIndex();

	//create descriptor heaps
	{
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
		heapDesc.NumDescriptors = FrameCount;
		heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

		ThrowIfFailed(m_Device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_heapDesc)));

		m_rtvDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	}

	//create frame resource

	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandler(m_heapDesc->GetCPUDescriptorHandleForHeapStart());

		for (UINT n = 0; n < FrameCount; n++)
		{
			ThrowIfFailed(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n])));
			m_Device->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandler);
			rtvHandler.Offset(1, m_rtvDescriptorSize);
		}
	};

	//command allocator
	ThrowIfFailed(m_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator)));
}

void DXBase::LoadAssets()
{
	ThrowIfFailed(m_Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_commandList)));

	ThrowIfFailed(m_commandList->Close());

	{
		m_Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence));
		m_fenceValue = 1;

		m_fenceEvent = CreateEvent(nullptr, false, false, nullptr);

		if (m_fenceEvent == nullptr)
		{
			ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
		}
	}

}

void DXBase::waitForPreviousFrame()
{
	// WAITING FOR THE FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.
	// This is code implemented as such for simplicity. The D3D12HelloFrameBuffering
	// sample illustrates how to use fences for efficient resource usage and to
	// maximize GPU utilization.

	// Signal and increment the fence value.
	const UINT Fence = m_fenceValue;
	ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), Fence));
	m_fenceValue++;

	//wait until previous frame completed
	if (m_fence->GetCompletedValue() < Fence)
	{
		ThrowIfFailed(m_fence->SetEventOnCompletion(Fence, m_fenceEvent));
		WaitForSingleObject(m_fenceEvent, INFINITE);
	}

	m_curentFrameIndex = m_swapChain->GetCurrentBackBufferIndex();
}

void DXBase::populateCommandList()
{
	// Command list allocators can only be reset when the associated 
	// command lists have finished execution on the GPU; apps should use 
	// fences to determine GPU execution progress.
	ThrowIfFailed(m_commandAllocator->Reset());

	// However, when ExecuteCommandList() is called on a particular command 
	// list, that command list can then be reset at any time and must be before 
	// re-recording.
	ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), m_pipelineState.Get()));

	// Indicate that the back buffer will be used as a render target.
	m_commandList->ResourceBarrier(1,
		&CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_curentFrameIndex].Get(),
			D3D12_RESOURCE_STATE_PRESENT,
			D3D12_RESOURCE_STATE_RENDER_TARGET
		)
	);

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandler(m_heapDesc->GetCPUDescriptorHandleForHeapStart(), m_curentFrameIndex, m_rtvDescriptorSize);

	//record comamnds
	const float clearColor[] = {0.0f, 0.2f, 0.4f, 1.0f};
	m_commandList->ClearRenderTargetView(rtvHandler, clearColor, 0, nullptr);

	// Indicate that the back buffer will now be used to present.
	m_commandList->ResourceBarrier(1,
		&CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_curentFrameIndex].Get(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PRESENT
		)
	);

	ThrowIfFailed(m_commandList->Close());
}