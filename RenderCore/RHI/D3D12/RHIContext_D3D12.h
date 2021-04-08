#pragma once
#include "../RHIContext.h"

#include <vector>

#include <d3d12.h>
#include <dxgi1_4.h>
#include <DirectXMath.h>
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

//include from engine core
//#include "../../EngineCore/d3dx12.h"

//#include "Uploadbuffer.h"

#include <string>
#include <wrl.h>

using namespace Microsoft::WRL;
using namespace DirectX;

class FRHIContext_D3D12 : public IRHIContext
{
public:
	FRHIContext_D3D12(){}
	~FRHIContext_D3D12(){}

	virtual void InitializeRHI(int InWidth, int InHeight)override;
	virtual void Resize(int InWidth, int InHeight)override;
	
private:
	void OnResize();
	void FlushCommandQueue();

	//for graphics
	int ClientWidth = 800;
	int ClientHeight = 600;

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

	//std::vector<std::unique_ptr<Geometry>> M_Geometies;

	////heap for cbv srv uav heap
	//ComPtr<ID3D12DescriptorHeap> M_CbvSrvUavHeap;

	////constant buffer
	//std::unique_ptr<UploadBuffer<ObjectConstants>> M_ConstantUploadBuffer = nullptr;

	////signature
	//ComPtr<ID3D12RootSignature> M_RootSignaure;

	////vs, ps
	//ComPtr<ID3DBlob> M_Vs;
	//ComPtr<ID3DBlob> M_Ps;
	//std::vector<D3D12_INPUT_ELEMENT_DESC> M_ShadersInputDesc;

	////pso
	//ComPtr<ID3D12PipelineState>	M_Pso;

	XMFLOAT4X4 IdendityMatrix = XMFLOAT4X4(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

};