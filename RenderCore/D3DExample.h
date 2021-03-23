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

#define  AssetPath L"Models/ModelSave.Bin"
#define  ShaderPath L"Shaders\\Test.hlsl"
#define WindowClass L"MainWnd"
#define  MenuName L"D3DEx"
#define WindowTitle L"D3D12Example"

struct Vertex
{
	 XMFLOAT3 Position;
	 XMFLOAT3 Normal;
	 XMFLOAT2 Uv;
};

struct ObjectConstants
{
	XMFLOAT4X4 WorldViewProj = DirectX::XMFLOAT4X4(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);

	XMFLOAT3 CameraLocation = XMFLOAT3(0.0f, 0.0f, 0.0f);
};

class Geometry
{
public:
	Geometry(ID3D12Device* Device, std::vector<Vertex>& Vertices, std::vector<uint32_t> Indices)
	{
		VertexBufferSize = sizeof(Vertex) * Vertices.size();
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
		IndexBufferSize = sizeof(uint32_t) * Indices.size();
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

	Geometry(const Geometry& rhs) = delete;
	Geometry& operator=(const Geometry& rhs) = delete;
	
	~Geometry()
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
	DXExample(HINSTANCE Instance) :AppInstan(Instance){
		M_ClientWidth = 800;
		M_ClientHeight = 600;
	}
	~DXExample() {}

	bool Initialize();
	bool Run();

	//for windo
	void InitWindow();
	//
	void InitializeDX();
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
		return M_BackBuffer[M_CurrentBackBuffer].Get();
	}

	D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentBackBufferView()
	{
		return CD3DX12_CPU_DESCRIPTOR_HANDLE(
			M_RtvHeap->GetCPUDescriptorHandleForHeapStart(),
			M_CurrentBackBuffer,
			M_RtvDescriptorSize);
	}

	D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentDepthStencilView()
	{
		return M_DsvHeap->GetCPUDescriptorHandleForHeapStart();
	}


private:
	HINSTANCE AppInstan = nullptr;
	HWND Mainhandle;


	//for graphics
	int M_ClientWidth = 800;
	int M_ClientHeight = 600;

	ComPtr<IDXGIFactory4> M_Factory;
	ComPtr<ID3D12Device> M_Device;
	ComPtr<IDXGISwapChain> M_SwapChain;

	ComPtr<ID3D12CommandAllocator> M_CommandAllocator;
	ComPtr<ID3D12CommandQueue> M_CommandQueue;
	ComPtr<ID3D12GraphicsCommandList>	M_CommandList;
	ComPtr<ID3D12Fence>	M_Fence;
	UINT64 M_CurrentFenceValue;

	ComPtr<ID3D12DescriptorHeap>	M_RtvHeap;
	ComPtr<ID3D12DescriptorHeap>	M_DsvHeap;
	static const int M_SwapchainBackbufferCount = 2;
	ComPtr<ID3D12Resource>	M_BackBuffer[M_SwapchainBackbufferCount];
	ComPtr<ID3D12Resource> M_DepthStencilBuffer;
	int M_CurrentBackBuffer = 0;
	
	UINT M_RtvDescriptorSize;
	UINT M_DsvDescriptorSize;
	UINT M_CbvSrvUavDescriptorSize;

	DXGI_FORMAT M_BackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT M_DepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	D3D12_VIEWPORT M_ScreenViewport;
	D3D12_RECT M_ScissorRect;

	std::vector<std::unique_ptr<Geometry>> M_Geometies;

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

	XMFLOAT4X4 IdendityMatrix = XMFLOAT4X4(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);
};
