#pragma once
#include "../RHIContext.h"

#include <vector>

#include <d3d12.h>
#include <dxgi1_4.h>
#include <DirectXMath.h>
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")


//#include "Uploadbuffer.h"

#include <string>
#include <wrl.h>

#include "d3dx12.h"

using namespace Microsoft::WRL;
using namespace DirectX;

#define  ShaderPathVS L"Shaders\\TestVS.hlsl"
#define  ShaderPathPS L"Shaders\\TestPS.hlsl"

class FRHIContext_D3D12 : public IRHIContext
{
public:
	FRHIContext_D3D12(){}
	virtual ~FRHIContext_D3D12();

	virtual void InitializeRHI(int InWidth, int InHeight)override;
	virtual void Resize(int InWidth, int InHeight)override;

	virtual IRHIGraphicsPipelineState* CreateGraphicsPSO()override; 
	virtual IRHIGraphicsPipelineState* CreateGraphicsDepthPSO()override;
	virtual IRHIGraphicsPipelineState* CreateSkinnedGraphicsPSO()override;

	// for populate commands
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
	
	virtual void BeginDraw(const wchar_t* Label)override;
	virtual void EndDraw()override;

	virtual void SetGraphicsPipilineState(IRHIGraphicsPipelineState* InPSO)override;

	virtual void SetViewport(const FViewport& Viewport)override;
	virtual void SetScissor(long InX, long InY, long InWidth, long InHeight)override;

	virtual void SetBackBufferAsRt()override;
	virtual void SetRenderTarget(IRHIResource* InColor, IRHIResource* InDepth)override;

	virtual void TransitionBackBufferStateToRT()override;
	virtual void TransitionBackBufferStateToPresent()override;
	virtual void TransitionResource(IRHIResource* InResource, ERHIResourceState StateBefore, ERHIResourceState StateAfter)override;

	virtual void PrepareShaderParameter()override;
	virtual void PrepareDepthShaderParameter()override;

	virtual void DrawRenderingMeshes(std::vector<IRHIRenderingMesh*>& Items)override;

	virtual void SetSceneConstantBuffer(IRHIConstantBuffer<FSceneConstant>* InBuffer)override;
	virtual void SetShadowMapSRV(FRHIDepthResource* InDepthResource)override;

	virtual void FlushCommandQueue()override;
	virtual void Present()override;

	virtual IRHIRenderingMesh* CreateEmptyRenderingMesh()override;

	virtual IRHIConstantBuffer<FSceneConstant>* CreateSceneConstantBuffer(const FSceneConstant& SceneConstant)override;

	ID3D12DescriptorHeap* GetCbvSrvUavDescriptorHeap() {
		return M_CbvSrvUavHeap.Get();
	}

	virtual FRHIDepthResource* CreateShadowDepthResource(int InWidth, int InHeight, EPixelBufferFormat InFormat)override;
	virtual void CreateSrvDsvForDepthResource(FRHIDepthResource* InDepthResource)override;

private:
	void OnResize();
	void BuildRootSignature();
	void BuildDepthRootSignature();
	void BuildDescriptorHeap();

	void BuildShadersInputLayout();

	D3D12_RESOURCE_STATES TranslateResourceState(ERHIResourceState InState);

	//for graphics
	int ClientWidth = 800;
	int ClientHeight = 600;

	ComPtr<IDXGIFactory4> M_Factory;
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

	//heap for cbv srv uav heap
	ComPtr<ID3D12DescriptorHeap> M_CbvSrvUavHeap;

	//signature
	ComPtr<ID3D12RootSignature> M_RootSignaure;
	ComPtr<ID3D12RootSignature> Depth_RootSignature;

	XMFLOAT4X4 IdendityMatrix = XMFLOAT4X4(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	//ShadersInput Layout
	std::vector<D3D12_INPUT_ELEMENT_DESC> ShadersInputDesc_Static;
	std::vector<D3D12_INPUT_ELEMENT_DESC> ShadersInputDesc_Skinned;

};