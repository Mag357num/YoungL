#include "pch.h"

#include "RHIContext_D3D12.h"
#include "RHIGraphicsPipelineState_D3D12.h"
#include "RHIShaderResource_D3D12.h"

#include "CompiledShaders/SkinnedMeshVS.h"
#include "CompiledShaders/SkinnedMeshPS.h"

#include <DirectXColors.h>

#include <D3Dcompiler.h>
#pragma comment(lib, "D3DCompiler.lib")
#include <pix.h>
#include <dxgidebug.h>

#include "RHIVertexBuffer_D3D12.h"
#include "RHIIndexBuffer_D3D12.h"
#include "RHIConstantBuffer_D3D12.h"
#include "RHIRenderingItem_D3D12.h"
#include "RHIDepthResource_D3D12.h"
#include "RHIResourceHandle_D3D12.h"

namespace WinApp
{
	extern HWND Mainhandle;
};

namespace D3D12RHI
{
	ComPtr<ID3D12Device> M_Device;
}

using namespace D3D12RHI;

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
	DsvHeapDesc.NumDescriptors = 2;//0: for scene color ;1 :for shadow depth

	M_Device->CreateDescriptorHeap(&DsvHeapDesc, IID_PPV_ARGS(&M_DsvHeap));

	//initialize back buffer
	OnResize();

	BuildRootSignature();
	//for depth root signature
	BuildDepthRootSignature();
	//for skinned mesh
	BuildSkinnedRootSignature();

	BuildDescriptorHeap();
	BuildShadersInputLayout();
}

FRHIContext_D3D12::~FRHIContext_D3D12()
{

	if (M_Device != nullptr)
	{
		FlushCommandQueue();
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

	//transilate dsv state to depth
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

//for render model
void FRHIContext_D3D12::BuildDescriptorHeap()
{
	D3D12_DESCRIPTOR_HEAP_DESC HeapDesc = {};
	HeapDesc.NumDescriptors = 1;
	HeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	HeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	M_Device->CreateDescriptorHeap(&HeapDesc, IID_PPV_ARGS(&M_CbvSrvUavHeap));
}


void FRHIContext_D3D12::BuildRootSignature()
{
	CD3DX12_ROOT_PARAMETER slotRootParameter[3];

	// Create a single descriptor table of CBVs.
	CD3DX12_DESCRIPTOR_RANGE cbvTable;
	cbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);
	slotRootParameter[0].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_ALL);
	slotRootParameter[1].InitAsConstantBufferView(1, 0, D3D12_SHADER_VISIBILITY_ALL);
	slotRootParameter[2].InitAsDescriptorTable(1, &cbvTable, D3D12_SHADER_VISIBILITY_ALL);

	// A root signature is an array of root parameters.
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(3, slotRootParameter, 0, nullptr,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	if (errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}

	M_Device->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(&M_RootSignaure));
}

void FRHIContext_D3D12::BuildDepthRootSignature()
{
	CD3DX12_ROOT_PARAMETER slotRootParameter[2];

	// Create a single descriptor table of CBVs.
	slotRootParameter[0].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_ALL);
	slotRootParameter[1].InitAsConstantBufferView(1, 0, D3D12_SHADER_VISIBILITY_ALL);

	// A root signature is an array of root parameters.
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(2, slotRootParameter, 0, nullptr,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	if (errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}

	M_Device->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(&Depth_RootSignature));
}

void FRHIContext_D3D12::BuildSkinnedRootSignature()
{
	CD3DX12_ROOT_PARAMETER slotRootParameter[4];

	// Create a single descriptor table of CBVs.
	CD3DX12_DESCRIPTOR_RANGE cbvTable;
	cbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);
	slotRootParameter[0].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_ALL);
	slotRootParameter[1].InitAsConstantBufferView(1, 0, D3D12_SHADER_VISIBILITY_ALL);
	slotRootParameter[2].InitAsDescriptorTable(1, &cbvTable, D3D12_SHADER_VISIBILITY_ALL);
	slotRootParameter[3].InitAsConstantBufferView(2, 0, D3D12_SHADER_VISIBILITY_ALL);

	// A root signature is an array of root parameters.
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(4, slotRootParameter, 0, nullptr,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	if (errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}

	M_Device->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(&Skinned_RootSignature));
}


void FRHIContext_D3D12::BuildShadersInputLayout()
{
	ShadersInputDesc_Static =
	{
		 { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		 { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		 { "TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	ShadersInputDesc_Skinned = 
	{
		 { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },//12
		 { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },//12
		 { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },//8

		 { "TAGANT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },//12
		 { "BONEINDEX", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, 44, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },//16
		 { "BONEWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 60, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }//16
	};
}

IRHIGraphicsPipelineState* FRHIContext_D3D12::CreateGraphicsPSO()
{
	FRHIGraphicsPipelineState_D3D12* D3D12GraphicsPSO = new FRHIGraphicsPipelineState_D3D12();
	//
	D3D12_GRAPHICS_PIPELINE_STATE_DESC Desc;
	ZeroMemory(&Desc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

	Desc.pRootSignature = M_RootSignaure.Get();
	Desc.InputLayout = { ShadersInputDesc_Static.data(), (UINT)ShadersInputDesc_Static.size() };
	Desc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	Desc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	Desc.RasterizerState.CullMode = D3D12_CULL_MODE_FRONT;

	D3D12_DEPTH_STENCIL_DESC DSDesc;
	DSDesc.DepthEnable = TRUE;
	DSDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	DSDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	DSDesc.StencilEnable = FALSE;
	DSDesc.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
	DSDesc.StencilReadMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
	DSDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	DSDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	DSDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	DSDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;

	DSDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	DSDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	DSDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	DSDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;

	//Desc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	Desc.DepthStencilState = DSDesc;

	Desc.VS =
	{
		g_BasePassVS,
		sizeof(g_BasePassVS)
	};
	Desc.PS =
	{
		g_BasePassPS,
		sizeof(g_BasePassPS)
	};

	Desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	Desc.NumRenderTargets = 1;
	Desc.RTVFormats[0] = M_BackBufferFormat;
	Desc.DSVFormat = M_DepthStencilFormat;
	Desc.SampleDesc.Count = 1;
	Desc.SampleDesc.Quality = 0;
	Desc.SampleMask = UINT_MAX;

	M_Device->CreateGraphicsPipelineState(&Desc, IID_PPV_ARGS(&D3D12GraphicsPSO->PSO));

	return D3D12GraphicsPSO;
}

IRHIGraphicsPipelineState* FRHIContext_D3D12::CreateGraphicsDepthPSO()
{
	FRHIGraphicsPipelineState_D3D12* D3D12GraphicsDepthPSO = new FRHIGraphicsPipelineState_D3D12();

	//
	D3D12_GRAPHICS_PIPELINE_STATE_DESC Desc;
	ZeroMemory(&Desc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

	Desc.pRootSignature = Depth_RootSignature.Get();
	Desc.InputLayout = { ShadersInputDesc_Static.data(), (UINT)ShadersInputDesc_Static.size() };
	Desc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	Desc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	Desc.RasterizerState.CullMode = D3D12_CULL_MODE_FRONT;

	D3D12_DEPTH_STENCIL_DESC DSDesc; 
	DSDesc.DepthEnable = TRUE;
	DSDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	DSDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	DSDesc.StencilEnable = FALSE;
	DSDesc.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
	DSDesc.StencilReadMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
	DSDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	DSDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	DSDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	DSDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;

	DSDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	DSDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	DSDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	DSDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;

	//Desc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	Desc.DepthStencilState = DSDesc;

	//Desc.RasterizerState.DepthBiasClamp = 0.0f;
	//Desc.RasterizerState.DepthBias = 100000;
	//Desc.RasterizerState.SlopeScaledDepthBias = 0.5f;

	Desc.VS =
	{
		g_DepthVS,
		sizeof(g_DepthVS)
	};
	Desc.PS =
	{
		g_DepthPS,
		sizeof(g_DepthPS)
	};

	Desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	Desc.NumRenderTargets = 1;
	Desc.RTVFormats[0] = M_BackBufferFormat;
	Desc.DSVFormat = M_DepthStencilFormat;
	Desc.SampleDesc.Count = 1;
	Desc.SampleDesc.Quality = 0;
	Desc.SampleMask = UINT_MAX;

	M_Device->CreateGraphicsPipelineState(&Desc, IID_PPV_ARGS(&D3D12GraphicsDepthPSO->PSO));

	return D3D12GraphicsDepthPSO;
}

IRHIGraphicsPipelineState* FRHIContext_D3D12::CreateSkinnedGraphicsPSO()
{
	FRHIGraphicsPipelineState_D3D12* D3D12GraphicsPSO = new FRHIGraphicsPipelineState_D3D12();

	//
	D3D12_GRAPHICS_PIPELINE_STATE_DESC Desc;
	ZeroMemory(&Desc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

	Desc.pRootSignature = Skinned_RootSignature.Get();
	Desc.InputLayout = { ShadersInputDesc_Skinned.data(), (UINT)ShadersInputDesc_Skinned.size() };
	Desc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	Desc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	Desc.RasterizerState.CullMode = D3D12_CULL_MODE_FRONT;

	D3D12_DEPTH_STENCIL_DESC DSDesc;
	DSDesc.DepthEnable = TRUE;
	DSDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	DSDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	DSDesc.StencilEnable = FALSE;
	DSDesc.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
	DSDesc.StencilReadMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
	DSDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	DSDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	DSDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	DSDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;

	DSDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	DSDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	DSDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	DSDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;

	//Desc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	Desc.DepthStencilState = DSDesc;

	Desc.VS =
	{
		g_SkinnedMeshVS,
		sizeof(g_SkinnedMeshVS)
	};
	Desc.PS =
	{
		g_SkinnedMeshPS,
		sizeof(g_SkinnedMeshPS)
	};

	Desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	Desc.NumRenderTargets = 1;
	Desc.RTVFormats[0] = M_BackBufferFormat;
	Desc.DSVFormat = M_DepthStencilFormat;
	Desc.SampleDesc.Count = 1;
	Desc.SampleDesc.Quality = 0;
	Desc.SampleMask = UINT_MAX;

	M_Device->CreateGraphicsPipelineState(&Desc, IID_PPV_ARGS(&D3D12GraphicsPSO->PSO));

	return D3D12GraphicsPSO;
}

void FRHIContext_D3D12::BeginDraw(const wchar_t* Label)
{
	M_CommandAllocator->Reset();
	M_CommandList->Reset(M_CommandAllocator.Get(), nullptr);

	::PIXBeginEvent(M_CommandList.Get(), PIX_COLOR_DEFAULT, Label);
}

void FRHIContext_D3D12::EndDraw()
{
	::PIXEndEvent(M_CommandList.Get());

	M_CommandList->Close();
	ID3D12CommandList* CmdLists[] = { M_CommandList.Get() };
	M_CommandQueue->ExecuteCommandLists(_countof(CmdLists), CmdLists);
}

void FRHIContext_D3D12::SetGraphicsPipilineState(IRHIGraphicsPipelineState* InPSO)
{
	FRHIGraphicsPipelineState_D3D12* D3D12PSO = reinterpret_cast<FRHIGraphicsPipelineState_D3D12*>(InPSO);
	M_CommandList->SetPipelineState(D3D12PSO->PSO.Get());
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

	M_CommandList->RSSetViewports(1, &VP);
}

void FRHIContext_D3D12::SetScissor(long InX, long InY, long InWidth, long InHeight)
{
	D3D12_RECT Rect;
	Rect.left = InX;
	Rect.top = InY;
	Rect.right = InWidth;
	Rect.bottom = InHeight;

	M_CommandList->RSSetScissorRects(1, &Rect);
}

void FRHIContext_D3D12::SetBackBufferAsRt()
{

	//clear target view
	M_CommandList->ClearRenderTargetView(GetCurrentBackBufferView(), Colors::Black, 0, nullptr);
	M_CommandList->ClearDepthStencilView(GetCurrentDepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	D3D12_CPU_DESCRIPTOR_HANDLE BackBufferHandle = GetCurrentBackBufferView();
	D3D12_CPU_DESCRIPTOR_HANDLE DsvrHandle = GetCurrentDepthStencilView();
	M_CommandList->OMSetRenderTargets(1, &BackBufferHandle, true, &DsvrHandle);
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
	case State_Srv:
		Ret = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
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
			M_CommandList->ClearDepthStencilView(*DsvHandle->GetCpuHandle(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

		}
	}
	
	//bool bHasRtv = false;
	//D3D12_CPU_DESCRIPTOR_HANDLE* Rtv = nullptr;

	//if (InColor && reinterpret_cast<FRHIResource_D3D12*>(InColor))
	//{
	//	FRHIResource_D3D12* ColorResource = reinterpret_cast<FRHIResource_D3D12*>(InColor);
	//	if (reinterpret_cast<FRHIResourceHandle_D3D12*>(ColorResource->))
	//	{
	//		FRHIResourceHandle_D3D12* DsvHandle = reinterpret_cast<FRHIResourceHandle_D3D12*>(DepthResource->GetDsvHandle());
	//		Dsv = DsvHandle->GetCpuHandle();
	//		M_CommandList->ClearDepthStencilView(*DsvHandle->GetCpuHandle(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	//	}
	//}

	
	M_CommandList->OMSetRenderTargets(0, nullptr, false, Dsv);
}

void FRHIContext_D3D12::TransitionBackBufferStateToRT()
{
	//change targe state
	D3D12_RESOURCE_BARRIER BarrierRT = CD3DX12_RESOURCE_BARRIER::Transition(GetCurrentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	M_CommandList->ResourceBarrier(1, &BarrierRT);
}

void FRHIContext_D3D12::TransitionBackBufferStateToPresent()
{
	D3D12_RESOURCE_BARRIER BarrierPresent = CD3DX12_RESOURCE_BARRIER::Transition(GetCurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	M_CommandList->ResourceBarrier(1, &BarrierPresent);
}

void FRHIContext_D3D12::TransitionResource(IRHIResource* InResource, ERHIResourceState StateBefore, ERHIResourceState StateAfter)
{
	if (!InResource)
	{
		return;
	}
	FRHIDepthResource_D3D12* Resource_D3D12 = reinterpret_cast<FRHIDepthResource_D3D12*>(InResource);
	if (!Resource_D3D12)
	{
		return;
	}

	D3D12_RESOURCE_STATES Before = TranslateResourceState(StateBefore);
	D3D12_RESOURCE_STATES After = TranslateResourceState(StateAfter);

	ID3D12Resource* NativeResource= Resource_D3D12->Resource.Get();
	D3D12_RESOURCE_BARRIER BarrierRT = CD3DX12_RESOURCE_BARRIER::Transition(NativeResource, Before, After);

	M_CommandList->ResourceBarrier(1, &BarrierRT);

}


void FRHIContext_D3D12::PrepareShaderParameter()
{
	ID3D12DescriptorHeap* DescriporHeaps[] = { M_CbvSrvUavHeap.Get() };
	M_CommandList->SetDescriptorHeaps(_countof(DescriporHeaps), DescriporHeaps);
	M_CommandList->SetGraphicsRootSignature(M_RootSignaure.Get());
}

void FRHIContext_D3D12::PrepareDepthShaderParameter()
{
	M_CommandList->SetGraphicsRootSignature(Depth_RootSignature.Get());
}

void FRHIContext_D3D12::PrepareSkinnedShaderParameter()
{
	M_CommandList->SetGraphicsRootSignature(Skinned_RootSignature.Get());
}

void FRHIContext_D3D12::SetSceneConstantBuffer(IRHIConstantBuffer<FSceneConstant>* InBuffer)
{
	FRHIConstantBuffer_D3D12<FSceneConstant>* Buffer_D3D12 = reinterpret_cast<FRHIConstantBuffer_D3D12<FSceneConstant>*>(InBuffer);
	M_CommandList->SetGraphicsRootConstantBufferView(Buffer_D3D12->GetRootParameterIndex(), Buffer_D3D12->GetGpuAddress());
}

void FRHIContext_D3D12::SetShadowMapSRV(FRHIDepthResource* InDepthResource)
{
	if (InDepthResource && InDepthResource->GetSrvHandle())
	{
		FRHIResourceHandle_D3D12*  SrvHandle = reinterpret_cast<FRHIResourceHandle_D3D12*>(InDepthResource->GetSrvHandle());
		M_CommandList->SetGraphicsRootDescriptorTable(2, *SrvHandle->GetGpuHandle());//root parameter index is 2
	}
}

void FRHIContext_D3D12::DrawRenderingMeshes(std::unordered_map<std::string, IRHIRenderingMesh*>& Items)
{
	for (auto It = Items.begin(); It != Items.end(); ++It)
	{
		FRHIVertexBuffer_D3D12* VertexBuffer = reinterpret_cast<FRHIVertexBuffer_D3D12*>(It->second->GetVertexBuffer());
		D3D12_VERTEX_BUFFER_VIEW VbView = VertexBuffer->GetVBView();
		M_CommandList->IASetVertexBuffers(0, 1, &VbView);

		FRHIIndexBuffer_D3D12* IndexBuffer = reinterpret_cast<FRHIIndexBuffer_D3D12*>(It->second->GetIndexBuffer());
		D3D12_INDEX_BUFFER_VIEW IbView = IndexBuffer->GetIBView();
		M_CommandList->IASetIndexBuffer(&IbView);
		M_CommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		FRHIConstantBuffer_D3D12<FObjectConstants>* ConstantBuffer = reinterpret_cast<FRHIConstantBuffer_D3D12<FObjectConstants>*>(It->second->GetConstantBuffer());
		M_CommandList->SetGraphicsRootConstantBufferView(ConstantBuffer->GetRootParameterIndex(), ConstantBuffer->GetGpuAddress());
		
		if (It->second->GetIsSkinned())
		{
			FRHIConstantBuffer_D3D12<FBoneTransforms>* BoneTransformsBuffer = reinterpret_cast<FRHIConstantBuffer_D3D12<FBoneTransforms>*>(It->second->GetBoneTransformsBuffer());
			M_CommandList->SetGraphicsRootConstantBufferView(BoneTransformsBuffer->GetRootParameterIndex(), BoneTransformsBuffer->GetGpuAddress());//root signature parameter: slot 3
		}
		
		M_CommandList->DrawIndexedInstanced((UINT)It->second->GetIndexCount(), 1, 0, 0, 0);

		
	}
	
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

void FRHIContext_D3D12::Present()
{
	M_SwapChain->Present(0, 0);
	M_CurrentBackBuffer = (M_CurrentBackBuffer + 1) % M_SwapchainBackbufferCount;
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

	ConstantBuffer->SetRootParameterIndex(1);//0 for Object constants 
	ConstantBuffer->SetGpuVirtualAddress(GpuAddress);

	return ConstantBuffer;
}

//create depth resoruce
FRHIDepthResource* FRHIContext_D3D12::CreateShadowDepthResource(int InWidth, int InHeight, EPixelBufferFormat InFormat)
{

	FRHIDepthResource_D3D12* DepthResource = new FRHIDepthResource_D3D12();

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
	ResourceDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	ResourceDesc.Height = InHeight;
	ResourceDesc.Width = InWidth;
	ResourceDesc.MipLevels = 1;
	ResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	ResourceDesc.SampleDesc.Count = 1;
	ResourceDesc.SampleDesc.Quality = 0;

	D3D12_CLEAR_VALUE ClearValue;
	ClearValue.Format = M_DepthStencilFormat;
	ClearValue.DepthStencil.Depth = 1.0f;
	ClearValue.DepthStencil.Stencil = 0;

	M_Device->CreateCommittedResource(&HeapProperties, 
			D3D12_HEAP_FLAG_NONE, &ResourceDesc, D3D12_RESOURCE_STATE_DEPTH_READ,
	&ClearValue, IID_PPV_ARGS(&DepthResource->Resource));


	return DepthResource;
}

void FRHIContext_D3D12::CreateSrvDsvForDepthResource(FRHIDepthResource* InDepthResource)
{
	FRHIDepthResource_D3D12* DepthResource_D3D12 = reinterpret_cast<FRHIDepthResource_D3D12*>(InDepthResource);

	D3D12_CPU_DESCRIPTOR_HANDLE SrvCpuDescriptorStart(M_CbvSrvUavHeap->GetCPUDescriptorHandleForHeapStart());

	D3D12_SHADER_RESOURCE_VIEW_DESC SrvDesc = {};
	SrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	SrvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	SrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	SrvDesc.Texture2D.MipLevels = 1;
	SrvDesc.Texture2D.MostDetailedMip = 0;
	SrvDesc.Texture2D.PlaneSlice = 0;
	SrvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

	M_Device->CreateShaderResourceView(DepthResource_D3D12->Resource.Get(), &SrvDesc, SrvCpuDescriptorStart);

	D3D12_GPU_DESCRIPTOR_HANDLE SrvGpuDescriptorStart(M_CbvSrvUavHeap->GetGPUDescriptorHandleForHeapStart());

	FRHIResourceHandle_D3D12* SrvHandle = new FRHIResourceHandle_D3D12();
	SrvHandle->SetCpuhandle(SrvCpuDescriptorStart);
	SrvHandle->SetGpuhandle(SrvGpuDescriptorStart);
	DepthResource_D3D12->SetSrvHandle(SrvHandle);

	D3D12_CPU_DESCRIPTOR_HANDLE DsvCpuDescriptorStart(M_DsvHeap->GetCPUDescriptorHandleForHeapStart());
	DsvCpuDescriptorStart.ptr += M_DsvDescriptorSize;//0 reserved for base pass depth
	D3D12_DEPTH_STENCIL_VIEW_DESC DsvDesc;
	DsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	DsvDesc.Format = M_DepthStencilFormat;
	DsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	DsvDesc.Texture2D.MipSlice = 0;

	M_Device->CreateDepthStencilView(DepthResource_D3D12->Resource.Get(), &DsvDesc, DsvCpuDescriptorStart);


	FRHIResourceHandle_D3D12* DsvHandle = new FRHIResourceHandle_D3D12();
	DsvHandle->SetCpuhandle(DsvCpuDescriptorStart);
	D3D12_GPU_DESCRIPTOR_HANDLE DsvGpuDescriptorStart(M_DsvHeap->GetGPUDescriptorHandleForHeapStart());
	DsvGpuDescriptorStart.ptr += M_DsvDescriptorSize;//0 reserved for base pass depth
	DsvHandle->SetGpuhandle(DsvGpuDescriptorStart);
	DepthResource_D3D12->SetDsvHandle(DsvHandle);
}