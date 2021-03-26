#include "D3DExample.h"
#include <fstream>
#include <DirectXColors.h>

#include "CompiledShaders/TestVS.h"
#include "CompiledShaders/TestPS.h"

#include <D3Dcompiler.h>
#pragma comment(lib, "D3DCompiler.lib")

bool DXExample::Initialize()
{
	InitWindow();

	InitializeDX();

	OnResize();

	BuildRootSignature();
	BuildDescriptorHeap();
	BuildConstantBuffers();
	BuildShadersInputLayout();
	BuildPsos();

	LoadAsset();

	return true;
}

bool DXExample::Run()
{
	MSG Msg = { 0 };

	while (Msg.message != WM_QUIT)
	{
		if (PeekMessage(&Msg, 0, 0, 0 , PM_REMOVE))
		{
			TranslateMessage(&Msg);
			DispatchMessage(&Msg);
		}
		else
		{
			Update();
			Render();
		}
	}

	return (int)Msg.wParam;
}

LRESULT CALLBACK
WndProc(HWND Hwnd, UINT Msg, WPARAM WPara, LPARAM LPara)
{
	switch (Msg)
	{
	case WM_QUIT:
		PostQuitMessage(0);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		break;
	}

	return DefWindowProc(Hwnd, Msg, WPara, LPara);
}

void DXExample::InitWindow()
{
	//window class
	WNDCLASS WDClass;
	WDClass.hInstance = AppInstan;
	WDClass.lpfnWndProc = WndProc;
	WDClass.cbClsExtra = 0;
	WDClass.cbWndExtra = 0;
	WDClass.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	WDClass.hCursor = LoadCursor(0, IDC_ARROW);
	WDClass.hIcon = LoadIcon(0, IDI_APPLICATION);
	WDClass.style = CS_HREDRAW | CS_VREDRAW;
	WDClass.lpszClassName = WindowClass;
	WDClass.lpszMenuName = MenuName;

	RegisterClass(&WDClass);
	//adjust, create window
	RECT Rec = {0, 0, M_ClientWidth, M_ClientHeight};
	AdjustWindowRect(&Rec, WS_OVERLAPPEDWINDOW, false);
	//int Width = Rec.right - Rec.left;
	//int Height = Rec.bottom - Rec.top;
	Mainhandle = CreateWindow(WindowClass, WindowTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, M_ClientWidth, M_ClientHeight, 0, 0, AppInstan, 0);

	//show window
	ShowWindow(Mainhandle, SW_SHOW);
	UpdateWindow(Mainhandle);

}

void DXExample::InitializeDX()
{
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
	SwapChainDesc.OutputWindow = Mainhandle;
	SwapChainDesc.Windowed = true;
	SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	SwapChainDesc.BufferDesc.Width = M_ClientWidth;
	SwapChainDesc.BufferDesc.Height = M_ClientHeight;
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
}

void DXExample::OnResize()
{
	FlushCommandQueue();

	//reset command list
	M_CommandList->Reset(M_CommandAllocator.Get(), nullptr);

	//reset back buffer;
	for (int i =0; i< M_SwapchainBackbufferCount; ++i)
	{
		M_BackBuffer[i].Reset();
	}
	M_DepthStencilBuffer.Reset();

	//resize swapchain
	M_SwapChain->ResizeBuffers(M_SwapchainBackbufferCount, M_ClientWidth, M_ClientHeight,
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
	DRDesc.Width = M_ClientWidth;
	DRDesc.Height = M_ClientHeight;
	DRDesc.DepthOrArraySize = 1;
	DRDesc.MipLevels = 1;

	// Correction 11/12/2016: SSAO chapter requires an SRV to the depth buffer to read from 
	// the depth buffer.  Therefore, because we need to create two views to the same resource:
	//   1. SRV format: DXGI_FORMAT_R24_UNORM_X8_TYPELESS
	//   2. DSV Format: DXGI_FORMAT_D24_UNORM_S8_UINT
	// we need to create the depth buffer resource with a typeless format.  
	DRDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;

	DRDesc.SampleDesc.Count = 1;
	DRDesc.SampleDesc.Quality =  0;
	DRDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	DRDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;


	D3D12_CLEAR_VALUE ClearValue;
	ClearValue.Format = M_DepthStencilFormat;
	ClearValue.DepthStencil.Depth = 1.0f;
	ClearValue.DepthStencil.Stencil = 0;

	M_Device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &DRDesc, D3D12_RESOURCE_STATE_COMMON,
		&ClearValue, IID_PPV_ARGS(&M_DepthStencilBuffer));

	CD3DX12_CPU_DESCRIPTOR_HANDLE DsvHandle(M_DsvHeap->GetCPUDescriptorHandleForHeapStart());
	D3D12_DEPTH_STENCIL_VIEW_DESC DsvDesc;
	DsvDesc.Format = M_DepthStencilFormat;
	DsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	DsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	DsvDesc.Texture2D.MipSlice = 0;
	M_Device->CreateDepthStencilView(M_DepthStencilBuffer.Get(), &DsvDesc, DsvHandle);

	//transilate dsv stato to depth
	M_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(M_DepthStencilBuffer.Get(), D3D12_RESOURCE_STATE_COMMON,
		D3D12_RESOURCE_STATE_DEPTH_WRITE));

	//excute command list
	M_CommandList->Close();
	ID3D12CommandList* cmdLists[] = {M_CommandList.Get()};
	M_CommandQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);
	
	//flush command queue
	FlushCommandQueue();

	//update screen view port
	M_ScreenViewport.Width = static_cast<float>(M_ClientWidth);
	M_ScreenViewport.Height = static_cast<float>(M_ClientHeight);
	M_ScreenViewport.TopLeftX = 0;
	M_ScreenViewport.TopLeftY = 0;
	M_ScreenViewport.MaxDepth = 1.0f;
	M_ScreenViewport.MinDepth = 0.0f;

	M_ScissorRect = { 0,0, M_ClientWidth, M_ClientHeight };
}

void DXExample::LoadAsset()
{
	
	//load asset
	std::vector<Vertex> Vertices;
	std::ifstream Fin(AssetPath, std::ios::in | std::ios::binary);

	int VertexNum;
	Fin.read((char*)&VertexNum, sizeof(int));
	Vertices.resize(VertexNum);
	Fin.read((char*)Vertices.data(), sizeof(Vertex) * VertexNum);

	std::vector<uint32_t> Indices;
	int IndexNum;
	Fin.read((char*)&IndexNum, sizeof(int));
	Indices.resize(IndexNum);
	Fin.read((char*)Indices.data(), sizeof(int) * IndexNum);

	Fin.close();

	//create resource
	std::unique_ptr<Geometry> Geo = std::make_unique<Geometry>(M_Device.Get(), Vertices, Indices);
	M_Geometies.push_back(std::move(Geo));

}

void DXExample::Update()
{
	XMFLOAT4X4 mWorld = IdendityMatrix;
	XMFLOAT4X4 mView = IdendityMatrix;

	XMFLOAT4X4 mProj = IdendityMatrix;

	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * 3.1416, (1.0f*M_ClientWidth/ M_ClientHeight), 1.0f, 1000.0f);
	XMStoreFloat4x4(&mProj, P);

	// Build the view matrix.
	XMVECTOR pos = XMVectorSet(500, 500, 100, 1.0f);
	XMVECTOR target = XMVectorSet(0, 0, 150, 0.0f);
	XMVECTOR up = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);

	XMMATRIX view = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&mView, view);

	XMMATRIX world = XMLoadFloat4x4(&mWorld);
	XMMATRIX proj = XMLoadFloat4x4(&mProj);
	XMMATRIX worldViewProj = world * view * proj;

	// Update the constant buffer with the latest worldViewProj matrix.
	ObjectConstants objConstants;
	XMStoreFloat4x4(&objConstants.WorldViewProj, XMMatrixTranspose(worldViewProj));
	XMStoreFloat3(&objConstants.CameraLocation, pos);
	M_ConstantUploadBuffer->CopyData(0, objConstants);
}

void DXExample::Render()
{
	PopulateCommands();
	FlushCommandQueue();
}

void DXExample::PopulateCommands()
{
	//reset
	M_CommandAllocator->Reset();
	M_CommandList->Reset(M_CommandAllocator.Get(), M_Pso.Get());
	
	//viewport
	M_CommandList->RSSetViewports(1, &M_ScreenViewport);
	M_CommandList->RSSetScissorRects(1, &M_ScissorRect);

	//change targe state
	M_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(GetCurrentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	//clear target view
	M_CommandList->ClearRenderTargetView(GetCurrentBackBufferView(), Colors::Black, 0, nullptr);
	M_CommandList->ClearDepthStencilView(GetCurrentDepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	M_CommandList->OMSetRenderTargets(1, &GetCurrentBackBufferView(), true, &GetCurrentDepthStencilView());

	//
	ID3D12DescriptorHeap* DescriporHeaps[] = {M_CbvSrvUavHeap.Get()};
	M_CommandList->SetDescriptorHeaps(_countof(DescriporHeaps), DescriporHeaps);
	M_CommandList->SetGraphicsRootSignature(M_RootSignaure.Get());
	
	M_CommandList->IASetVertexBuffers(0, 1, &M_Geometies[0]->GetVertexBufferView());
	M_CommandList->IASetIndexBuffer(&M_Geometies[0]->GetIndexBufferView());
	M_CommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	M_CommandList->SetGraphicsRootDescriptorTable(0, M_CbvSrvUavHeap->GetGPUDescriptorHandleForHeapStart());

	M_CommandList->DrawIndexedInstanced(M_Geometies[0]->IndexCount, 1, 0, 0, 0);

	//change targe state
	M_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(GetCurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	M_CommandList->Close();
	ID3D12CommandList* CmdLists[] = { M_CommandList.Get()};
	M_CommandQueue->ExecuteCommandLists(_countof(CmdLists), CmdLists);

	M_SwapChain->Present(0, 0);

	M_CurrentBackBuffer = (M_CurrentBackBuffer + 1) % M_SwapchainBackbufferCount;
}

void DXExample::FlushCommandQueue()
{
	M_CurrentFenceValue++;
	
	M_CommandQueue->Signal(M_Fence.Get(), M_CurrentFenceValue);

	if (M_Fence->GetCompletedValue() < M_CurrentFenceValue)
	{
		HANDLE EventHandle = CreateEventEx(nullptr, nullptr,false, EVENT_ALL_ACCESS);
		M_Fence->SetEventOnCompletion(M_CurrentFenceValue, EventHandle);

		WaitForSingleObject(EventHandle, INFINITE);
		CloseHandle(EventHandle);
	}
}

void DXExample::BuildRootSignature()
{
	CD3DX12_ROOT_PARAMETER slotRootParameter[1];

	// Create a single descriptor table of CBVs.
	CD3DX12_DESCRIPTOR_RANGE cbvTable;
	cbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	slotRootParameter[0].InitAsDescriptorTable(1, &cbvTable);

	// A root signature is an array of root parameters.
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(1, slotRootParameter, 0, nullptr,
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

//for render model
void DXExample::BuildDescriptorHeap()
{
	D3D12_DESCRIPTOR_HEAP_DESC HeapDesc;
	HeapDesc.NumDescriptors = 1;
	HeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	HeapDesc.NodeMask = 0;
	HeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	M_Device->CreateDescriptorHeap(&HeapDesc, IID_PPV_ARGS(&M_CbvSrvUavHeap));
}

void DXExample::BuildShadersInputLayout()
{
	UINT CompileFlags = 0;

#if defined(DEBUG) || defined(_DEBUG)  
	CompileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	ComPtr<ID3DBlob> CompileError;
	D3DCompileFromFile(ShaderPathVS, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_0", CompileFlags, 0, &M_Vs, &CompileError);

	if (CompileError != nullptr)
	{
		OutputDebugStringA((char*)CompileError->GetBufferPointer());
	}

	D3DCompileFromFile(ShaderPathPS, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_0", CompileFlags, 0, &M_Ps, &CompileError);
	if (CompileError != nullptr)
	{
		OutputDebugStringA((char*)CompileError->GetBufferPointer());
	}

	M_ShadersInputDesc =
	{
		 { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		 { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		 { "TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

}

void DXExample::BuildPsos()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC Desc;
	ZeroMemory(&Desc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

	Desc.pRootSignature = M_RootSignaure.Get();
	Desc.InputLayout = { M_ShadersInputDesc.data(), (UINT)M_ShadersInputDesc.size()};
	Desc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	Desc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	Desc.RasterizerState.CullMode = D3D12_CULL_MODE_FRONT;
	Desc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	//Desc.VS =
	//{
	//	reinterpret_cast<BYTE*>(M_Vs->GetBufferPointer()),
	//	M_Vs->GetBufferSize()
	//};
	//Desc.PS =
	//{
	//	reinterpret_cast<BYTE*>(M_Ps->GetBufferPointer()),
	//	M_Ps->GetBufferSize()
	//};

	Desc.VS =
	{
		g_TestVS,
		sizeof(g_TestVS)
	};
	Desc.PS =
	{
		g_TestPS,
		sizeof(g_TestPS)
	};

	Desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	Desc.NumRenderTargets = 1;
	Desc.RTVFormats[0] = M_BackBufferFormat;
	Desc.DSVFormat = M_DepthStencilFormat;
	Desc.SampleDesc.Count = 1;
	Desc.SampleDesc.Quality = 0;
	Desc.SampleMask = UINT_MAX;

	M_Device->CreateGraphicsPipelineState(&Desc, IID_PPV_ARGS(&M_Pso));
}

void DXExample::BuildConstantBuffers()
{
	M_ConstantUploadBuffer = std::make_unique<UploadBuffer<ObjectConstants>>(M_Device.Get(), 1, true);
	UINT ObjectBufferSize = UploadBuffer<ObjectConstants>::CalcConstantBufferByteSize(sizeof(ObjectConstants));

	D3D12_CPU_DESCRIPTOR_HANDLE CpuDescriptor = M_CbvSrvUavHeap->GetCPUDescriptorHandleForHeapStart();
	D3D12_CONSTANT_BUFFER_VIEW_DESC ViewDesc;
	
	D3D12_GPU_VIRTUAL_ADDRESS GpuAddress = M_ConstantUploadBuffer->GetResource()->GetGPUVirtualAddress();
	int BufIndex = 0;
	GpuAddress += BufIndex * ObjectBufferSize;
	ViewDesc.BufferLocation = GpuAddress;
	ViewDesc.SizeInBytes = ObjectBufferSize;

	M_Device->CreateConstantBufferView(&ViewDesc, CpuDescriptor);
}

