#pragma once

#include <d3d12.h>
#include <dxgi1_4.h>
#include <DirectXMath.h>
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

//include from engine core
#include "../EngineCore/d3dx12.h"

#include "Uploadbuffer.h"

#include <string>
#include <wrl.h>

using namespace Microsoft::WRL;
using namespace DirectX;

struct Vertex
{
	 XMFLOAT3 Position;
	 XMFLOAT4 Color;
};

struct ObjectConstants
{
	XMFLOAT4X4 WorldViewProj = DirectX::XMFLOAT4X4(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);
};

class DrawGeometry
{
public:
	DrawGeometry(ID3D12Device* Device, std::vector<Vertex>& Vertices, std::vector<uint32_t> Indices) 
	{
		VertexBufferSize = sizeof(Vertices);
		VertexStrideSize = sizeof(Vertex);
		Device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(VertexBufferSize), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
			IID_PPV_ARGS(&M_VertexBuffer));

		UINT8* VertexBegin;
		D3D12_RANGE VtCopyRange;
		M_VertexBuffer->Map(0, &VtCopyRange, reinterpret_cast<void**>(&VertexBegin));
		memcpy(VertexBegin, Vertices.data(), VertexBufferSize);
		M_VertexBuffer->Unmap(0, nullptr);


		//create index buffer view
		IndexBufferSize = sizeof(Indices);
		Device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(IndexBufferSize), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
			IID_PPV_ARGS(&M_IndexBuffer));

		UINT8* IndexBegin;
		D3D12_RANGE IdCopyRange;
		M_IndexBuffer->Map(0, &IdCopyRange, reinterpret_cast<void**>(&IndexBegin));
		memcpy(IndexBegin, Indices.data(), IndexBufferSize);
		M_IndexBuffer->Unmap(0, nullptr);

		IndexCount = Indices.size();
	}

	DrawGeometry(const DrawGeometry& rhs) = delete;
	DrawGeometry& operator=(const DrawGeometry& rhs) = delete;
	
	~DrawGeometry()
	{
	}


public:
	//vertex buffer&& index buffer
	ComPtr<ID3D12Resource> M_VertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView()
	{
		D3D12_VERTEX_BUFFER_VIEW Desc;
		Desc.BufferLocation = M_VertexBuffer->GetGPUVirtualAddress();
		Desc.SizeInBytes = VertexBufferSize;
		Desc.StrideInBytes = sizeof(Vertex);

		return Desc;
	}

	ComPtr<ID3D12Resource> M_IndexBuffer;;
	D3D12_INDEX_BUFFER_VIEW GetIndexBufferView()
	{
		D3D12_INDEX_BUFFER_VIEW Desc;
		Desc.BufferLocation = M_IndexBuffer->GetGPUVirtualAddress();
		Desc.SizeInBytes = IndexBufferSize;
		Desc.Format = DXGI_FORMAT_R32_UINT;

		return Desc;
	}

	int VertexStrideSize = 0;
	int VertexBufferSize = 0;
	int IndexBufferSize = 0;
	size_t IndexCount = 0;
	int InstanceCount = 0;
	int VertexBaseLocation = 0;
	int StartInstanceLocation = 0;


};

class DXExample
{
public:
	DXExample(HINSTANCE Instance) :AppInstan(Instance){}
	~DXExample() {}

	bool Initialize();
	bool Run();

	//for windo
	void InitWindow();
	//
	void LoadPipline();
	void OnResize();
	void LoadAsset();

	//for model rendering
	void BuildDescriptorHeap();
	void BuildConstantBuffers();

	void BuildRootSignature();
	void BuildShadersInputLayout();
	void BuildPsos();
	

	//for graphics
	void Update();
	void Render();

	void PopulateCommands();
	void FlushCommandQueue();


	//for populate commands
	ID3D12Resource* GetCurrentBackBuffer() {
		return m_BackBuffer[M_CurrentBackBuffer].Get();
	}

	D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentBackBufferView()
	{
		return CD3DX12_CPU_DESCRIPTOR_HANDLE(
			RtvHeap->GetCPUDescriptorHandleForHeapStart(),
			M_CurrentBackBuffer,
			RtvDescriptorSize);
	}

	D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentDepthStencilView()
	{
		return DsvHeap->GetCPUDescriptorHandleForHeapStart();
	}


private:
	HINSTANCE AppInstan = nullptr;
	HWND Mainhandle;
	std::wstring mMainWndCaption = L"d3d App";


	//for graphics
	int ClientWidth = 800;
	int ClientHeight = 600;

	ComPtr<IDXGIFactory4> m_Factory;
	ComPtr<ID3D12Device> m_Device;
	ComPtr<IDXGISwapChain> m_SwapChain;

	ComPtr<ID3D12CommandAllocator> m_CommandAllocator;
	ComPtr<ID3D12CommandQueue> m_CommandQueue;
	ComPtr<ID3D12GraphicsCommandList>	m_CommandList;
	ComPtr<ID3D12Fence>	m_Fence;
	UINT64 m_CurrentFenceValue;

	ComPtr<ID3D12DescriptorHeap>	RtvHeap;
	ComPtr<ID3D12DescriptorHeap>	DsvHeap;
	static const int m_SwapchainBackbufferCount = 2;
	ComPtr<ID3D12Resource>	m_BackBuffer[m_SwapchainBackbufferCount];
	ComPtr<ID3D12Resource> m_DepthStencilBuffer;
	int M_CurrentBackBuffer = 0;
	
	UINT RtvDescriptorSize;
	UINT DsvDescriptorSize;
	UINT CbvSrvUavDescriptorSize;

	DXGI_FORMAT BackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT DepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	D3D12_VIEWPORT M_ScreenViewport;
	D3D12_RECT M_ScissorRect;

	std::vector<std::unique_ptr<DrawGeometry>> Geometies;

	//heap for cbv srv uav heap
	ComPtr<ID3D12DescriptorHeap> M_CbvSrvUavHeap;

	//constant buffer
	std::unique_ptr<UploadBuffer<ObjectConstants>> M_ConstantUploadBuffer = nullptr;

	//signature
	ComPtr<ID3D12RootSignature> M_RootSignaure;

	//vs, ps
	ComPtr<ID3DBlob> M_Vs;
	ComPtr<ID3DBlob> M_Ps;
	std::vector<D3D12_INPUT_ELEMENT_DESC> M_ShadersInputDesc;

	//pso
	ComPtr<ID3D12PipelineState>	M_Pso;
};
