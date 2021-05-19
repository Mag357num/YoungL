#include "pch.h"

#include "RHIContext_D3D12.h"
#include "RHIGraphicsPipelineState_D3D12.h"
#include "RHIComputePipelineState_D3D12.h"
#include "RHIShaderResource_D3D12.h"

#include <DirectXColors.h>

#include <pix.h>
#include <dxgidebug.h>

#include "RHIVertexBuffer_D3D12.h"
#include "RHIIndexBuffer_D3D12.h"
#include "RHIConstantBuffer_D3D12.h"
#include "RHIRenderingItem_D3D12.h"
#include "RHIDepthResource_D3D12.h"
#include "RHIResourceHandle_D3D12.h"
#include "RHIColorResource_D3D12.h"

namespace WinApp
{
	extern HWND Mainhandle;
};

namespace D3D12RHI
{
	ComPtr<ID3D12Device> M_Device;
}

using namespace D3D12RHI;

static const uint32_t NumDescriptorsPerHeap = 256;

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
	CreateDXGIFactory1(IID_PPV_ARGS(&Factory));

	//enumerate adapter device
	//try to create hard ware device
	HRESULT Result = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&M_Device));

	if (FAILED(Result))
	{
		Microsoft::WRL::ComPtr<IDXGIAdapter> WarpAdapter;
		Factory->EnumWarpAdapter(IID_PPV_ARGS(&WarpAdapter));
		D3D12CreateDevice(WarpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&M_Device));
	}

	RtvDescriptorSize = M_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	DsvDescriptorSize = M_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	CbvSrvUavDescriptorSize = M_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	//create commang objects
	M_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&CommandAllocator));
	D3D12_COMMAND_QUEUE_DESC QueueDesc = {};
	QueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	QueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	M_Device->CreateCommandQueue(&QueueDesc, IID_PPV_ARGS(&CommandQueue));
	M_Device->CreateCommandList(1, D3D12_COMMAND_LIST_TYPE_DIRECT, CommandAllocator.Get(), nullptr, IID_PPV_ARGS(&CommandList));

	CommandList->Close();

	//create fence
	M_Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&Fence));

	//create swapchain
	SwapChain.Reset();
	DXGI_SWAP_CHAIN_DESC SwapChainDesc;
	SwapChainDesc.BufferCount = SwapchainBackbufferCount;
	SwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	SwapChainDesc.SampleDesc.Count = 1;
	SwapChainDesc.SampleDesc.Quality = 0;
	SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	SwapChainDesc.OutputWindow = WinApp::Mainhandle;
	SwapChainDesc.Windowed = true;
	SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	SwapChainDesc.BufferDesc.Width = ClientWidth;
	SwapChainDesc.BufferDesc.Height = ClientHeight;
	SwapChainDesc.BufferDesc.Format = BackBufferFormat;
	SwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	SwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	SwapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_CENTERED;
	SwapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;


	Factory->CreateSwapChain(CommandQueue.Get(), &SwapChainDesc, SwapChain.GetAddressOf());

	//create discriptor heap
	D3D12_DESCRIPTOR_HEAP_DESC RtvHeapDesc;
	RtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	RtvHeapDesc.NumDescriptors = NumDescriptorsPerHeap;//swapchain count for back buffer; another is for scenecolor(postprocess)
	RtvHeapDesc.NodeMask = 0;
	RtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	M_Device->CreateDescriptorHeap(&RtvHeapDesc, IID_PPV_ARGS(&RtvHeap));

	D3D12_DESCRIPTOR_HEAP_DESC DsvHeapDesc;
	DsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	DsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	DsvHeapDesc.NodeMask = 0;
	DsvHeapDesc.NumDescriptors = NumDescriptorsPerHeap;//0: for scene color ;1 :for shadow depth

	M_Device->CreateDescriptorHeap(&DsvHeapDesc, IID_PPV_ARGS(&DsvHeap));

	//initialize back buffer
	OnResize();
	
	//temperarily
	//todo:seperate rootsignature & shader Binding;
	//todo: seperate DescriptorHeap management;
	{

		BuildDescriptorHeap();
		//for postprocess heap
		PostProcess_BuildDescriptorHeap();
	}

	//initialize dyanmic allocated descriptor heap
	RtvDHAllocatedCount = 2; //reserved for back buffer target
	DsvDHAllocatedCount = 1; //reserved for ds view
	CbvDHAllocatedCount = 0;
	Present_CbvDHAllocatedCount = 0;

	//init shader Resource 
	ShaderResource = new FRHIShaderResource_D3D12();
}

FRHIContext_D3D12::~FRHIContext_D3D12()
{

	if (M_Device != nullptr)
	{
		FlushCommandQueue();
	}


	if (ShaderResource)
	{
		delete ShaderResource;
		ShaderResource = nullptr;
	}

//#if defined(DBUG) || defined(_DEBUG)
//	{
//		ID3D12DebugDevice* pDebugDevice = NULL;
//		M_Device->QueryInterface(&pDebugDevice);
//		pDebugDevice->ReportLiveDeviceObjects(D3D12_RLDO_DETAIL);
//		pDebugDevice->Release();
//	}
//#endif
	
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
	CommandList->Reset(CommandAllocator.Get(), nullptr);

	//reset back buffer;
	for (int i = 0; i < SwapchainBackbufferCount; ++i)
	{
		BackBuffer[i].Reset();
	}
	DepthStencilBuffer.Reset();

	//resize swapchain
	SwapChain->ResizeBuffers(SwapchainBackbufferCount, ClientWidth, ClientHeight,
		BackBufferFormat, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);

	CurrentBackBuffer = 0;

	//get back buffer from swap chain && create rtv
	CD3DX12_CPU_DESCRIPTOR_HANDLE Rtvhandle(RtvHeap->GetCPUDescriptorHandleForHeapStart());
	for (int i = 0; i < SwapchainBackbufferCount; ++i)
	{
		SwapChain->GetBuffer(i, IID_PPV_ARGS(&BackBuffer[i]));
		M_Device->CreateRenderTargetView(BackBuffer[i].Get(), nullptr, Rtvhandle);
		Rtvhandle.Offset(1, RtvDescriptorSize);
	}
	//

	//create depth staencil buffer and dsv
	D3D12_RESOURCE_DESC DRDesc;
	DRDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	DRDesc.Alignment = 0;
	DRDesc.Width = ClientWidth;
	DRDesc.Height = ClientHeight;
	DRDesc.DepthOrArraySize = 1;
	DRDesc.MipLevels = 1;

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
	ClearValue.Format = DepthStencilFormat;
	ClearValue.DepthStencil.Depth = 1.0f;
	ClearValue.DepthStencil.Stencil = 0;
	CD3DX12_HEAP_PROPERTIES HeapProperty(D3D12_HEAP_TYPE_DEFAULT);
	M_Device->CreateCommittedResource(&HeapProperty, D3D12_HEAP_FLAG_NONE, &DRDesc, D3D12_RESOURCE_STATE_COMMON,
		&ClearValue, IID_PPV_ARGS(&DepthStencilBuffer));

	CD3DX12_CPU_DESCRIPTOR_HANDLE DsvHandle(DsvHeap->GetCPUDescriptorHandleForHeapStart());
	D3D12_DEPTH_STENCIL_VIEW_DESC DsvDesc;
	DsvDesc.Format = DepthStencilFormat;
	DsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	DsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	DsvDesc.Texture2D.MipSlice = 0;
	M_Device->CreateDepthStencilView(DepthStencilBuffer.Get(), &DsvDesc, DsvHandle);

	//transilate dsv state to depth
	CD3DX12_RESOURCE_BARRIER Barrier = CD3DX12_RESOURCE_BARRIER::Transition(DepthStencilBuffer.Get(), D3D12_RESOURCE_STATE_COMMON,
		D3D12_RESOURCE_STATE_DEPTH_WRITE);
	CommandList->ResourceBarrier(1, &Barrier);

	//excute command list
	CommandList->Close();
	ID3D12CommandList* cmdLists[] = { CommandList.Get() };
	CommandQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);

	//flush command queue
	FlushCommandQueue();

	//update screen view port
	ScreenViewport.Width = static_cast<float>(ClientWidth);
	ScreenViewport.Height = static_cast<float>(ClientHeight);
	ScreenViewport.TopLeftX = 0;
	ScreenViewport.TopLeftY = 0;
	ScreenViewport.MaxDepth = 1.0f;
	ScreenViewport.MinDepth = 0.0f;

	ScissorRect = { 0,0, ClientWidth, ClientHeight };
}

//for render model
void FRHIContext_D3D12::BuildDescriptorHeap()
{
	D3D12_DESCRIPTOR_HEAP_DESC HeapDesc = {};
	HeapDesc.NumDescriptors = NumDescriptorsPerHeap;
	HeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	HeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	M_Device->CreateDescriptorHeap(&HeapDesc, IID_PPV_ARGS(&CbvSrvUavHeap));
}

void FRHIContext_D3D12::PostProcess_BuildDescriptorHeap()
{
	D3D12_DESCRIPTOR_HEAP_DESC HeapDesc = {};
	HeapDesc.NumDescriptors = NumDescriptorsPerHeap;
	HeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	HeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	M_Device->CreateDescriptorHeap(&HeapDesc, IID_PPV_ARGS(&Present_CbvSrvUavHeap));
}

IRHIGraphicsPipelineState* FRHIContext_D3D12::CreateEmptyGraphicsPSO()
{
	FRHIGraphicsPipelineState_D3D12* D3D12GraphicsPSO = new FRHIGraphicsPipelineState_D3D12();
	return D3D12GraphicsPSO;
}

IRHIComputePipelineState* FRHIContext_D3D12::CreateEmptyComputePSO()
{
	FRHIComputePipelineState_D3D12* D3D12ComputePSO = new FRHIComputePipelineState_D3D12();
	return D3D12ComputePSO;
}

void FRHIContext_D3D12::BeginDraw(const wchar_t* Label)
{
	CommandAllocator->Reset();
	CommandList->Reset(CommandAllocator.Get(), nullptr);

	::PIXBeginEvent(CommandList.Get(), PIX_COLOR_DEFAULT, Label);
}

void FRHIContext_D3D12::EndDraw()
{
	::PIXEndEvent(CommandList.Get());

	CommandList->Close();
	ID3D12CommandList* CmdLists[] = { CommandList.Get() };
	CommandQueue->ExecuteCommandLists(_countof(CmdLists), CmdLists);
}

void FRHIContext_D3D12::BeginEvent(const wchar_t* Label)
{
	::PIXBeginEvent(CommandList.Get(), PIX_COLOR_DEFAULT, Label);
}

void FRHIContext_D3D12::EndEvent()
{
	::PIXEndEvent(CommandList.Get());
}

void FRHIContext_D3D12::SetGraphicsPipilineState(IRHIGraphicsPipelineState* InPSO)
{
	FRHIGraphicsPipelineState_D3D12* D3D12PSO = reinterpret_cast<FRHIGraphicsPipelineState_D3D12*>(InPSO);
	CommandList->SetPipelineState(D3D12PSO->PSO.Get());
	CommandList->SetGraphicsRootSignature(D3D12PSO->RootSignature.Get());
}

void FRHIContext_D3D12::SetViewport(const FViewport& Viewport)
{
	D3D12_VIEWPORT VP;
	VP.Width = (float)Viewport.Width;
	VP.Height = (float)Viewport.Height;
	VP.TopLeftX = (float)Viewport.X;
	VP.TopLeftY = (float)Viewport.Y;
	VP.MaxDepth = (float)Viewport.MaxDepth;
	VP.MinDepth = (float)Viewport.MinDepth;

	CommandList->RSSetViewports(1, &VP);
}

void FRHIContext_D3D12::SetScissor(long InX, long InY, long InWidth, long InHeight)
{
	D3D12_RECT Rect;
	Rect.left = InX;
	Rect.top = InY;
	Rect.right = InWidth;
	Rect.bottom = InHeight;

	CommandList->RSSetScissorRects(1, &Rect);
}

void FRHIContext_D3D12::SetBackBufferAsRt()
{

	//clear target view
	CommandList->ClearRenderTargetView(GetCurrentBackBufferView(), Colors::LightBlue, 0, nullptr);
	CommandList->ClearDepthStencilView(GetCurrentDepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	D3D12_CPU_DESCRIPTOR_HANDLE BackBufferHandle = GetCurrentBackBufferView();
	D3D12_CPU_DESCRIPTOR_HANDLE DsvrHandle = GetCurrentDepthStencilView();
	CommandList->OMSetRenderTargets(1, &BackBufferHandle, true, &DsvrHandle);
}

D3D12_RESOURCE_STATES FRHIContext_D3D12::TranslateResourceState(ERHIResourceState InState)
{
	D3D12_RESOURCE_STATES Ret = D3D12_RESOURCE_STATE_COMMON;
	
	switch (InState)
	{
	case State_None:
		Ret = D3D12_RESOURCE_STATE_COMMON;
		break;
	case State_Present:
		Ret = D3D12_RESOURCE_STATE_PRESENT;
		break;
	case State_RenderTarget:
		Ret = D3D12_RESOURCE_STATE_RENDER_TARGET;
		break;
	case State_GenerateRead:
		Ret = D3D12_RESOURCE_STATE_GENERIC_READ;
		break;
	case State_Uav:
		Ret = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		break;
	case State_DepthRead:
		Ret = D3D12_RESOURCE_STATE_DEPTH_READ;
		break;
	case State_DepthWrite:
		Ret = D3D12_RESOURCE_STATE_DEPTH_WRITE;
		break;
	default:
		break;
	}

	return Ret;
}

void FRHIContext_D3D12::SetRenderTarget(IRHIResource* InColor, IRHIResource* InDepth)
{
	//deal with depth target
	D3D12_CPU_DESCRIPTOR_HANDLE* Dsv = nullptr;
	if (InDepth && reinterpret_cast<FRHIDepthResource_D3D12*>(InDepth))
	{
		FRHIDepthResource_D3D12* DepthResource = reinterpret_cast<FRHIDepthResource_D3D12*>(InDepth);
		if (reinterpret_cast<FRHIResourceHandle_D3D12*>(DepthResource->GetDsvHandle()))
		{
			FRHIResourceHandle_D3D12* DsvHandle = reinterpret_cast<FRHIResourceHandle_D3D12*>(DepthResource->GetDsvHandle());
			Dsv = DsvHandle->GetCpuHandle();
			CommandList->ClearDepthStencilView(*DsvHandle->GetCpuHandle(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
			
		}
	}
	
	bool bHasRtv = false;
	D3D12_CPU_DESCRIPTOR_HANDLE* Rtv = nullptr;
	if (InColor && reinterpret_cast<FRHIColorResource_D3D12*>(InColor))
	{
		FRHIColorResource_D3D12* ColorResource_D3D12 = reinterpret_cast<FRHIColorResource_D3D12*>(InColor);
		if (ColorResource_D3D12 && reinterpret_cast<FRHIResourceHandle_D3D12*>(ColorResource_D3D12->GetRTVHandle()))
		{
			bHasRtv = true;
			FRHIResourceHandle_D3D12* RtvHandle = reinterpret_cast<FRHIResourceHandle_D3D12*>(ColorResource_D3D12->GetRTVHandle());
			Rtv = RtvHandle->GetCpuHandle();
			CommandList->ClearRenderTargetView(*RtvHandle->GetCpuHandle(), Colors::LightBlue, 0, nullptr);
		}
	}

	if (!bHasRtv)
	{
		CommandList->OMSetRenderTargets(0, nullptr, false, Dsv);
	}
	else
	{
		CommandList->OMSetRenderTargets(1, Rtv, true, Dsv);
	}
	
}

void FRHIContext_D3D12::SetColorTarget(IRHIResource* InColor)
{
	//todl:output warning if nullptr

	if (InColor && reinterpret_cast<FRHIColorResource_D3D12*>(InColor))
	{
		FRHIColorResource_D3D12* ColorResource_D3D12 = reinterpret_cast<FRHIColorResource_D3D12*>(InColor);
		if (ColorResource_D3D12 && reinterpret_cast<FRHIResourceHandle_D3D12*>(ColorResource_D3D12->GetRTVHandle()))
		{
			FRHIResourceHandle_D3D12* RtvHandle = reinterpret_cast<FRHIResourceHandle_D3D12*>(ColorResource_D3D12->GetRTVHandle());

			CommandList->ClearRenderTargetView(*RtvHandle->GetCpuHandle(), ColorResource_D3D12->GetClearValue().Color, 0, nullptr);

			CommandList->ClearDepthStencilView(GetCurrentDepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

			D3D12_CPU_DESCRIPTOR_HANDLE DsvrHandle = GetCurrentDepthStencilView();
			CommandList->OMSetRenderTargets(1, RtvHandle->GetCpuHandle(), true, &DsvrHandle);
		}
	}
}

void FRHIContext_D3D12::TransitionBackBufferStateToRT()
{
	//change targe state
	D3D12_RESOURCE_BARRIER BarrierRT = CD3DX12_RESOURCE_BARRIER::Transition(GetCurrentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	CommandList->ResourceBarrier(1, &BarrierRT);
}

void FRHIContext_D3D12::TransitionBackBufferStateToPresent()
{
	D3D12_RESOURCE_BARRIER BarrierPresent = CD3DX12_RESOURCE_BARRIER::Transition(GetCurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	CommandList->ResourceBarrier(1, &BarrierPresent);
}

void FRHIContext_D3D12::TransitionResource(IRHIResource* InResource, ERHIResourceState StateBefore, ERHIResourceState StateAfter)
{
	if (!InResource)
	{
		return;
	}


	D3D12_RESOURCE_STATES Before = TranslateResourceState(StateBefore);
	D3D12_RESOURCE_STATES After = TranslateResourceState(StateAfter);

	ID3D12Resource* NativeResource = nullptr;
	
	if (InResource->GetIsDepth())
	{
		FRHIDepthResource_D3D12* DepthResource_D3D12 = reinterpret_cast<FRHIDepthResource_D3D12*>(InResource);
		NativeResource = DepthResource_D3D12->Resource.Get();
	}
	else
	{
		FRHIColorResource_D3D12* ColorResource_D3D12 = reinterpret_cast<FRHIColorResource_D3D12*>(InResource);
		NativeResource = ColorResource_D3D12->Resource.Get();
	}
	
	D3D12_RESOURCE_BARRIER BarrierRT = CD3DX12_RESOURCE_BARRIER::Transition(NativeResource, Before, After);

	CommandList->ResourceBarrier(1, &BarrierRT);

}

D3D_PRIMITIVE_TOPOLOGY TranslateTopology(EPrimitiveTopology Topology)
{
	D3D_PRIMITIVE_TOPOLOGY Ret = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	switch (Topology)
	{
	case PrimitiveTopology_TRIANGLELIST:
		Ret = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		break;
	default:
		break;
	}

	return Ret;
}


void FRHIContext_D3D12::SetPrimitiveTopology(EPrimitiveTopology Topology)
{
	CommandList->IASetPrimitiveTopology(TranslateTopology(Topology));
}

void FRHIContext_D3D12::SetIndexBuffer(IRHIIndexBuffer* IndexBuffer)
{
	FRHIIndexBuffer_D3D12* IndexBuffer_D3D = reinterpret_cast<FRHIIndexBuffer_D3D12*>(IndexBuffer);
	D3D12_INDEX_BUFFER_VIEW IbView = IndexBuffer_D3D->GetIBView();
	CommandList->IASetIndexBuffer(&IbView);
}

void FRHIContext_D3D12::SetVertexBuffer(UINT StartSlot, UINT NumViews, IRHIVertexBuffer* VertexBuffer)
{
	FRHIVertexBuffer_D3D12* VertexBuffer_D3D = reinterpret_cast<FRHIVertexBuffer_D3D12*>(VertexBuffer);
	D3D12_VERTEX_BUFFER_VIEW VbView = VertexBuffer_D3D->GetVBView();
	CommandList->IASetVertexBuffers(StartSlot, NumViews, &VbView);
}

void FRHIContext_D3D12::SetInstanceVertexBuffer(UINT StartSlot, IRHIVertexBuffer* VertexBuffer, IRHIVertexBuffer* InstanceBuffer)
{
	D3D12_VERTEX_BUFFER_VIEW Views[2];
	FRHIVertexBuffer_D3D12* VertexBuffer_D3D = reinterpret_cast<FRHIVertexBuffer_D3D12*>(VertexBuffer);
	Views[0] = VertexBuffer_D3D->GetVBView();

	FRHIVertexBuffer_D3D12* InstanceBuffer_D3D = reinterpret_cast<FRHIVertexBuffer_D3D12*>(InstanceBuffer);
	Views[1] = InstanceBuffer_D3D->GetVBView();

	CommandList->IASetVertexBuffers(StartSlot, 2, Views);
}


void FRHIContext_D3D12::PrepareShaderParameter()
{
	ID3D12DescriptorHeap* DescriporHeaps[] = { CbvSrvUavHeap.Get() };
	CommandList->SetDescriptorHeaps(_countof(DescriporHeaps), DescriporHeaps);
	//M_CommandList->SetGraphicsRootSignature(M_RootSignaure.Get());
}

void FRHIContext_D3D12::PreparePresentShaderParameter()
{
	ID3D12DescriptorHeap* DescriporHeaps[] = { Present_CbvSrvUavHeap.Get() };
	CommandList->SetDescriptorHeaps(_countof(DescriporHeaps), DescriporHeaps);

	//M_CommandList->SetGraphicsRootSignature(Present_RootSignature.Get());
}


void FRHIContext_D3D12::SetGraphicRootConstant(UINT SlotParaIndex, UINT SrcData, UINT DestOffsetIn32BitValues)
{
	CommandList->SetGraphicsRoot32BitConstant(SlotParaIndex, SrcData, DestOffsetIn32BitValues);
}

void FRHIContext_D3D12::SetSceneConstantBufferView(UINT SlotParaIndex, IRHIConstantBuffer<FSceneConstant>* InBuffer)
{
	FRHIConstantBuffer_D3D12<FSceneConstant>* Buffer_D3D12 = reinterpret_cast<FRHIConstantBuffer_D3D12<FSceneConstant>*>(InBuffer);
	CommandList->SetGraphicsRootConstantBufferView(SlotParaIndex, Buffer_D3D12->GetGpuAddress());
}

void FRHIContext_D3D12::SetObjectConstantBufferView(UINT SlotParaIndex, IRHIConstantBuffer<FObjectConstants>* InBuffer)
{
	FRHIConstantBuffer_D3D12<FObjectConstants>* Buffer_D3D12 = reinterpret_cast<FRHIConstantBuffer_D3D12<FObjectConstants>*>(InBuffer);
	CommandList->SetGraphicsRootConstantBufferView(SlotParaIndex, Buffer_D3D12->GetGpuAddress());
}

void FRHIContext_D3D12::SetBoneTransformConstantBufferView(UINT SlotParaIndex, IRHIConstantBuffer<FBoneTransforms>* InBuffer)
{
	FRHIConstantBuffer_D3D12<FBoneTransforms>* Buffer_D3D12 = reinterpret_cast<FRHIConstantBuffer_D3D12<FBoneTransforms>*>(InBuffer);
	CommandList->SetGraphicsRootConstantBufferView(SlotParaIndex, Buffer_D3D12->GetGpuAddress());
}


void FRHIContext_D3D12::SetDepthAsSRV(UINT ParaIndex, FRHIDepthResource* InDepthResource)
{
	if (InDepthResource && InDepthResource->GetSrvHandle())
	{
		FRHIResourceHandle_D3D12*  SrvHandle = reinterpret_cast<FRHIResourceHandle_D3D12*>(InDepthResource->GetSrvHandle());
		CommandList->SetGraphicsRootDescriptorTable(ParaIndex, *SrvHandle->GetGpuHandle());//root parameter index is 2
	}
}

void FRHIContext_D3D12::SetColorSRV(UINT ParaIndex, FRHIColorResource* InColorResource)
{
	if (InColorResource && InColorResource->GetSrvHandle())
	{
		FRHIResourceHandle_D3D12* SrvHandle = reinterpret_cast<FRHIResourceHandle_D3D12*>(InColorResource->GetSrvHandle());
		CommandList->SetGraphicsRootDescriptorTable(ParaIndex, *SrvHandle->GetGpuHandle());
	}
}


void FRHIContext_D3D12::DrawIndexedInstanced(UINT IndexCountPerInstance, UINT IndexCount, 
						UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation)
{
	CommandList->DrawIndexedInstanced(IndexCountPerInstance, IndexCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
}

void FRHIContext_D3D12::Draw(UINT VertexCount, UINT VertexStartOffset)
{
	CommandList->DrawInstanced(VertexCount, 1, VertexStartOffset, 0);
}

void FRHIContext_D3D12::FlushCommandQueue()
{
	CurrentFenceValue++;

	CommandQueue->Signal(Fence.Get(), CurrentFenceValue);

	if (Fence->GetCompletedValue() < CurrentFenceValue)
	{
		HANDLE EventHandle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);
		Fence->SetEventOnCompletion(CurrentFenceValue, EventHandle);

		WaitForSingleObject(EventHandle, INFINITE);
		CloseHandle(EventHandle);
	}
}

void FRHIContext_D3D12::Present()
{
	SwapChain->Present(0, 0);
	CurrentBackBuffer = (CurrentBackBuffer + 1) % SwapchainBackbufferCount;
}

IRHIRenderingMesh* FRHIContext_D3D12::CreateEmptyRenderingMesh()
{
	return new FRHIRenderingMesh_D3D12();
}

IRHIConstantBuffer<FSceneConstant>* FRHIContext_D3D12::CreateSceneConstantBuffer(const FSceneConstant& SceneConstant)
{
	FRHIConstantBuffer_D3D12<FSceneConstant>* ConstantBuffer = new FRHIConstantBuffer_D3D12<FSceneConstant>();

	ConstantBuffer->UploadBuffer = std::make_unique<FRHIUploadBuffer_D3D12<FSceneConstant>>(new FRHIUploadBuffer_D3D12<FSceneConstant>(true));
	ConstantBuffer->UploadBuffer->CreateUploadResource(1);

	UINT ObjectBufferSize = FRHIUploadBuffer_D3D12<FSceneConstant>::CalcConstantBufferByteSize(sizeof(FSceneConstant));
	ConstantBuffer->CopyData(0, SceneConstant);

	FRHIResource_D3D12* UploadResource_D3D12 = reinterpret_cast<FRHIResource_D3D12*>(ConstantBuffer->UploadBuffer->GetResource());
	D3D12_GPU_VIRTUAL_ADDRESS GpuAddress = UploadResource_D3D12->Resource->GetGPUVirtualAddress();
	ConstantBuffer->SetGpuVirtualAddress(GpuAddress);

	return ConstantBuffer;
}

//create depth resoruce
FRHIDepthResource* FRHIContext_D3D12::CreateDepthResource(int InWidth, int InHeight, EPixelBufferFormat InFormat)
{

	FRHIDepthResource_D3D12* DepthResource = new FRHIDepthResource_D3D12(InWidth, InHeight, InFormat);

	CD3DX12_HEAP_PROPERTIES HeapProperties(D3D12_HEAP_TYPE_DEFAULT);

	D3D12_RESOURCE_DESC ResourceDesc;
	ZeroMemory(&ResourceDesc, sizeof(D3D12_RESOURCE_DESC));
	ResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	ResourceDesc.Alignment = 0;
	ResourceDesc.DepthOrArraySize = 1;
	ResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	// the depth buffer.  Therefore, because we need to create two views to the same resource:
	//   1. SRV format: DXGI_FORMAT_R24_UNORM_X8_TYPELESS
	//   2. DSV Format: DXGI_FORMAT_D24_UNORM_S8_UINT
	// we need to create the depth buffer resource with a typeless format. 
	ResourceDesc.Format = FRHIResource_D3D12::TranslateFormat(DepthResource->GetFormat());;
	ResourceDesc.Height = DepthResource->GetHeight();
	ResourceDesc.Width = DepthResource->GetWidth();
	ResourceDesc.MipLevels = 1;
	ResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	ResourceDesc.SampleDesc.Count = 1;
	ResourceDesc.SampleDesc.Quality = 0;

	D3D12_CLEAR_VALUE ClearValue;
	ClearValue.Format = DepthStencilFormat;
	ClearValue.DepthStencil.Depth = 1.0f;
	ClearValue.DepthStencil.Stencil = 0;

	M_Device->CreateCommittedResource(&HeapProperties, 
			D3D12_HEAP_FLAG_NONE, &ResourceDesc, D3D12_RESOURCE_STATE_DEPTH_READ,
	&ClearValue, IID_PPV_ARGS(&DepthResource->Resource));


	return DepthResource;
}

void FRHIContext_D3D12::CreateSrvDsvForDepthResource(FRHIDepthResource* InDepthResource)
{

	//TODO: if allocated count is outof descriptor range
	//if (CbvDHAllocatedCount == NumDescriptorsPerHeap)
	//{
	//}


	FRHIDepthResource_D3D12* DepthResource_D3D12 = reinterpret_cast<FRHIDepthResource_D3D12*>(InDepthResource);

	D3D12_CPU_DESCRIPTOR_HANDLE SrvCpuDescriptorStart(CbvSrvUavHeap->GetCPUDescriptorHandleForHeapStart());

	SrvCpuDescriptorStart.ptr += CbvDHAllocatedCount * CbvSrvUavDescriptorSize;

	D3D12_SHADER_RESOURCE_VIEW_DESC SrvDesc = {};
	SrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	SrvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	SrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	SrvDesc.Texture2D.MipLevels = 1;
	SrvDesc.Texture2D.MostDetailedMip = 0;
	SrvDesc.Texture2D.PlaneSlice = 0;
	SrvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

	M_Device->CreateShaderResourceView(DepthResource_D3D12->Resource.Get(), &SrvDesc, SrvCpuDescriptorStart);

	D3D12_GPU_DESCRIPTOR_HANDLE SrvGpuDescriptorStart(CbvSrvUavHeap->GetGPUDescriptorHandleForHeapStart());

	SrvGpuDescriptorStart.ptr += CbvDHAllocatedCount * CbvSrvUavDescriptorSize;

	FRHIResourceHandle_D3D12* SrvHandle = new FRHIResourceHandle_D3D12();
	SrvHandle->SetCpuhandle(SrvCpuDescriptorStart);
	SrvHandle->SetGpuhandle(SrvGpuDescriptorStart);
	DepthResource_D3D12->SetSrvHandle(SrvHandle);

	//incre cbv descriptor coupnt
	CbvDHAllocatedCount ++;

	D3D12_CPU_DESCRIPTOR_HANDLE DsvCpuDescriptorStart(DsvHeap->GetCPUDescriptorHandleForHeapStart());
	DsvCpuDescriptorStart.ptr += DsvDHAllocatedCount * DsvDescriptorSize;//0 reserved for base pass depth
	D3D12_DEPTH_STENCIL_VIEW_DESC DsvDesc;
	DsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	DsvDesc.Format = DepthStencilFormat;
	DsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	DsvDesc.Texture2D.MipSlice = 0;

	M_Device->CreateDepthStencilView(DepthResource_D3D12->Resource.Get(), &DsvDesc, DsvCpuDescriptorStart);


	FRHIResourceHandle_D3D12* DsvHandle = new FRHIResourceHandle_D3D12();
	DsvHandle->SetCpuhandle(DsvCpuDescriptorStart);
	D3D12_GPU_DESCRIPTOR_HANDLE DsvGpuDescriptorStart(DsvHeap->GetGPUDescriptorHandleForHeapStart());
	DsvGpuDescriptorStart.ptr += DsvDHAllocatedCount * DsvDescriptorSize;//0 reserved for base pass depth
	DsvHandle->SetGpuhandle(DsvGpuDescriptorStart);
	DepthResource_D3D12->SetDsvHandle(DsvHandle);

	//incre dsv heap
	DsvDHAllocatedCount ++;
}


FRHIColorResource* FRHIContext_D3D12::CreateColorResource(FColorResourceDesc Desc)
{
	FRHIColorResource_D3D12* ColorResource = new FRHIColorResource_D3D12(Desc.Width, Desc.Height, Desc.Format);

	CD3DX12_HEAP_PROPERTIES HeapProperties(D3D12_HEAP_TYPE_DEFAULT);

	D3D12_RESOURCE_DESC ResourceDesc;
	ZeroMemory(&ResourceDesc, sizeof(D3D12_RESOURCE_DESC));
	ResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	ResourceDesc.Alignment = 0;
	ResourceDesc.DepthOrArraySize = 1;
	ResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	ResourceDesc.Flags = FRHIResource_D3D12::TranslateResourceFlag(Desc.ResourceFlag);

	ResourceDesc.Format = FRHIResource_D3D12::TranslateFormat(ColorResource->GetFormat());
	ResourceDesc.Height = ColorResource->GetHeight();
	ResourceDesc.Width = ColorResource->GetWidth();
	ResourceDesc.MipLevels = 1;
	ResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	ResourceDesc.SampleDesc.Count = 1;
	ResourceDesc.SampleDesc.Quality = 0;

	D3D12_CLEAR_VALUE ClearValue;
	ClearValue.Format = FRHIResource_D3D12::TranslateFormat(ColorResource->GetFormat());
	//ClearValue light blue
	ClearValue.Color[0] = 0.678431392f;
	ClearValue.Color[1] = 0.847058892f;
	ClearValue.Color[2] = 0.901960850f;
	ClearValue.Color[3] = 1.0f;//0.678431392f, 0.847058892f, 0.901960850f

	ColorResource->SetClearValue(ClearValue);

	M_Device->CreateCommittedResource(&HeapProperties,
		D3D12_HEAP_FLAG_NONE, &ResourceDesc, TranslateResourceState(Desc.ResourceState),
		&ClearValue, IID_PPV_ARGS(&ColorResource->Resource));


	return ColorResource;
}

void FRHIContext_D3D12::CreateSrvRtvForColorResource(FRHIColorResource* InColorResource)
{
	//TODO: if allocated count is outof descriptor range
	//if (Present_CbvDHAllocatedCount == NumDescriptorsPerHeap)
	//{
	//}

	FRHIColorResource_D3D12* ColorResource_D3D12 = reinterpret_cast<FRHIColorResource_D3D12*>(InColorResource);

	D3D12_CPU_DESCRIPTOR_HANDLE SrvCpuDescriptorStart(Present_CbvSrvUavHeap->GetCPUDescriptorHandleForHeapStart());//todo  add postprocess descriptor
	SrvCpuDescriptorStart.ptr += Present_CbvDHAllocatedCount * CbvSrvUavDescriptorSize;

	D3D12_SHADER_RESOURCE_VIEW_DESC SrvDesc = {};
	SrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	SrvDesc.Format = FRHIResource_D3D12::TranslateFormat(InColorResource->GetFormat());
	SrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	SrvDesc.Texture2D.MipLevels = 1;
	SrvDesc.Texture2D.MostDetailedMip = 0;
	SrvDesc.Texture2D.PlaneSlice = 0;
	SrvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

	M_Device->CreateShaderResourceView(ColorResource_D3D12->Resource.Get(), &SrvDesc, SrvCpuDescriptorStart);

	D3D12_GPU_DESCRIPTOR_HANDLE SrvGpuDescriptorStart(Present_CbvSrvUavHeap->GetGPUDescriptorHandleForHeapStart());
	SrvGpuDescriptorStart.ptr += Present_CbvDHAllocatedCount * CbvSrvUavDescriptorSize;

	FRHIResourceHandle_D3D12* SrvHandle = new FRHIResourceHandle_D3D12();
	SrvHandle->SetCpuhandle(SrvCpuDescriptorStart);
	SrvHandle->SetGpuhandle(SrvGpuDescriptorStart);
	ColorResource_D3D12->SetSrvHandle(SrvHandle);

	//incre Present cbv allocator
	Present_CbvDHAllocatedCount++;


	D3D12_CPU_DESCRIPTOR_HANDLE RtvCpuDescriptorStart(RtvHeap->GetCPUDescriptorHandleForHeapStart());
	RtvCpuDescriptorStart.ptr += (RtvDHAllocatedCount * RtvDescriptorSize);//0, 1: reserved for base pass swap chaine rendertareget

	M_Device->CreateRenderTargetView(ColorResource_D3D12->Resource.Get(), nullptr, RtvCpuDescriptorStart);


	FRHIResourceHandle_D3D12* RtvHandle = new FRHIResourceHandle_D3D12();
	RtvHandle->SetCpuhandle(RtvCpuDescriptorStart);
	D3D12_GPU_DESCRIPTOR_HANDLE RtvGpuDescriptorStart(RtvHeap->GetGPUDescriptorHandleForHeapStart());
	RtvGpuDescriptorStart.ptr += (RtvDHAllocatedCount * RtvDescriptorSize);//0, 1: reserved for base pass swap chaine rendertareget
	RtvHandle->SetGpuhandle(RtvGpuDescriptorStart);
	ColorResource_D3D12->SetRtvHandle(RtvHandle);

	//incre count
	RtvDHAllocatedCount++;
}

void FRHIContext_D3D12::CreateSrvForColorResource(FRHIColorResource* InColorResource, bool ShouldCreateUAV)
{
	FRHIColorResource_D3D12* ColorResource_D3D12 = reinterpret_cast<FRHIColorResource_D3D12*>(InColorResource);

	D3D12_CPU_DESCRIPTOR_HANDLE SrvCpuDescriptorStart(CbvSrvUavHeap->GetCPUDescriptorHandleForHeapStart());//todo  add postprocess descriptor
	SrvCpuDescriptorStart.ptr += CbvDHAllocatedCount * CbvSrvUavDescriptorSize;

	D3D12_SHADER_RESOURCE_VIEW_DESC SrvDesc = {};
	SrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	SrvDesc.Format = FRHIResource_D3D12::TranslateFormat(InColorResource->GetFormat());
	SrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	SrvDesc.Texture2D.MipLevels = 1;
	SrvDesc.Texture2D.MostDetailedMip = 0;
	SrvDesc.Texture2D.PlaneSlice = 0;
	SrvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

	M_Device->CreateShaderResourceView(ColorResource_D3D12->Resource.Get(), &SrvDesc, SrvCpuDescriptorStart);

	D3D12_GPU_DESCRIPTOR_HANDLE SrvGpuDescriptorStart(CbvSrvUavHeap->GetGPUDescriptorHandleForHeapStart());
	SrvGpuDescriptorStart.ptr += CbvDHAllocatedCount * CbvSrvUavDescriptorSize;

	FRHIResourceHandle_D3D12* SrvHandle = new FRHIResourceHandle_D3D12();
	SrvHandle->SetCpuhandle(SrvCpuDescriptorStart);
	SrvHandle->SetGpuhandle(SrvGpuDescriptorStart);
	ColorResource_D3D12->SetSrvHandle(SrvHandle);

	//incre Present cbv allocator
	CbvDHAllocatedCount++;

	if (ShouldCreateUAV)
	{
		D3D12_CPU_DESCRIPTOR_HANDLE UavCpuDescriptorStart(CbvSrvUavHeap->GetCPUDescriptorHandleForHeapStart());//todo  add postprocess descriptor
		UavCpuDescriptorStart.ptr += CbvDHAllocatedCount * CbvSrvUavDescriptorSize;

		D3D12_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {};
		UAVDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
		UAVDesc.Format = FRHIResource_D3D12::TranslateFormat(InColorResource->GetFormat());
		UAVDesc.Texture2D.MipSlice = 0;
		//UAVDesc.Texture2D.PlaneSlice = 0;

		M_Device->CreateUnorderedAccessView(ColorResource_D3D12->Resource.Get(), nullptr, &UAVDesc, UavCpuDescriptorStart);

		D3D12_GPU_DESCRIPTOR_HANDLE UavGpuDescriptorStart(CbvSrvUavHeap->GetGPUDescriptorHandleForHeapStart());
		UavGpuDescriptorStart.ptr += CbvDHAllocatedCount * CbvSrvUavDescriptorSize;

		FRHIResourceHandle_D3D12* UavHandle = new FRHIResourceHandle_D3D12();
		UavHandle->SetCpuhandle(UavCpuDescriptorStart);
		UavHandle->SetGpuhandle(UavGpuDescriptorStart);
		ColorResource_D3D12->SetUAVHandle(UavHandle);

		//incre Present cbv allocator
		CbvDHAllocatedCount++;
	}

}

//TODO:Copy Data to render resource
void FRHIContext_D3D12::CopyTextureDataToResource(std::vector<FColor>& Colors, UINT Width, UINT Height, FRHIColorResource* ColorResource)
{
	{
		FRHIColorResource_D3D12* ColorResource_D3D12 = reinterpret_cast<FRHIColorResource_D3D12*>(ColorResource);
	
		const UINT64 uploadBufferSize = GetRequiredIntermediateSize(ColorResource_D3D12->Resource.Get(), 0, 1);

		// Create the GPU upload buffer.
		if (!ColorResource_D3D12->UploadResource.Get())
		{
			CD3DX12_HEAP_PROPERTIES HeapProperties(D3D12_HEAP_TYPE_UPLOAD);
			CD3DX12_RESOURCE_DESC ResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);
			HRESULT Hr = M_Device->CreateCommittedResource(
				&HeapProperties,
				D3D12_HEAP_FLAG_NONE,
				&ResourceDesc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(&ColorResource_D3D12->UploadResource));
		}
		
		// Copy data to the intermediate upload heap and then schedule a copy 
		// from the upload heap to the Texture2D.
		D3D12_SUBRESOURCE_DATA TextureData = {};
		TextureData.pData = Colors.data();
		TextureData.RowPitch = Width * sizeof(FColor);
		TextureData.SlicePitch = TextureData.RowPitch * Height;

		UpdateSubresources(CommandList.Get(), ColorResource_D3D12->Resource.Get(), ColorResource_D3D12->UploadResource.Get(), 0, 0, 1, &TextureData);

	}
}