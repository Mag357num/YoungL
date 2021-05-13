#pragma once
#include "../RHIContext.h"
#include "RHIShaderResource_D3D12.h"

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

class FRHIContext_D3D12 : public IRHIContext
{
public:
	FRHIContext_D3D12(){}
	virtual ~FRHIContext_D3D12();

	virtual void InitializeRHI(int InWidth, int InHeight)override;
	virtual void Resize(int InWidth, int InHeight)override;

	virtual IRHIGraphicsPipelineState* CreateEmpltyGraphicsPSO()override;


	// for populate commands
	ID3D12Resource* GetCurrentBackBuffer() {
		return BackBuffer[CurrentBackBuffer].Get();
	}

	D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentBackBufferView()
	{
		return CD3DX12_CPU_DESCRIPTOR_HANDLE(
			RtvHeap->GetCPUDescriptorHandleForHeapStart(),
			CurrentBackBuffer,
			RtvDescriptorSize);
	}

	D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentDepthStencilView()
	{
		return DsvHeap->GetCPUDescriptorHandleForHeapStart();
	}
	
	virtual void BeginDraw(const wchar_t* Label)override;
	virtual void EndDraw()override;

	virtual void BeginEvent(const wchar_t* Label)override;
	virtual void EndEvent()override;

	virtual void SetGraphicsPipilineState(IRHIGraphicsPipelineState* InPSO)override;

	virtual void SetViewport(const FViewport& Viewport)override;
	virtual void SetScissor(long InX, long InY, long InWidth, long InHeight)override;

	virtual void SetBackBufferAsRt()override;
	virtual void SetRenderTarget(IRHIResource* InColor, IRHIResource* InDepth)override;
	virtual void SetColorTarget(IRHIResource* InColor)override;

	virtual void TransitionBackBufferStateToRT()override;
	virtual void TransitionBackBufferStateToPresent()override;
	virtual void TransitionResource(IRHIResource* InResource, ERHIResourceState StateBefore, ERHIResourceState StateAfter)override;

	virtual void PrepareShaderParameter()override;
	virtual void PreparePresentShaderParameter()override;


	//for draw call info
	virtual void SetVertexBuffer(UINT StartSlot, UINT NumViews, IRHIVertexBuffer* VertexBuffer)override;
	virtual void SetIndexBuffer(IRHIIndexBuffer* IndexBuffer)override;
	virtual void SetPrimitiveTopology(EPrimitiveTopology Topology)override;


	virtual void DrawIndexedInstanced(UINT IndexCountPerInstance, UINT IndexCount, 
					UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation)override;
	virtual void Draw(UINT VertexCount, UINT VertexStartOffset /* = 0 */)override;


	virtual void SetGraphicRootConstant(UINT SlotParaIndex, UINT SrcData, UINT DestOffsetIn32BitValues)override;
	virtual void SetSceneConstantBufferView(UINT SlotParaIndex, IRHIConstantBuffer<FSceneConstant>* InBuffer)override;
	virtual void SetObjectConstantBufferView(UINT SlotParaIndex, IRHIConstantBuffer<FObjectConstants>* InBuffer)override;
	virtual void SetBoneTransformConstantBufferView(UINT SlotParaIndex, IRHIConstantBuffer<FBoneTransforms>* InBuffer)override;


	virtual void SetDepthAsSRV(UINT ParaIndex, FRHIDepthResource* InDepthResource)override;
	virtual void SetColorSRV(UINT ParaIndex, FRHIColorResource* InColorResource)override;

	virtual void FlushCommandQueue()override;
	virtual void Present()override;

	virtual IRHIRenderingMesh* CreateEmptyRenderingMesh()override;

	virtual IRHIConstantBuffer<FSceneConstant>* CreateSceneConstantBuffer(const FSceneConstant& SceneConstant)override;

	ID3D12DescriptorHeap* GetCbvSrvUavDescriptorHeap() {
		return CbvSrvUavHeap.Get();
	}

	virtual FRHIDepthResource* CreateDepthResource(int InWidth, int InHeight, EPixelBufferFormat InFormat)override;
	virtual void CreateSrvDsvForDepthResource(FRHIDepthResource* InDepthResource)override;

	virtual FRHIColorResource* CreateColorResource(int InWidth, int InHeight, EPixelBufferFormat InFormat)override;
	virtual void CreateSrvRtvForColorResource(FRHIColorResource* InColorResource)override;
	virtual void CreateSrvForColorResource(FRHIColorResource* InColorResource)override;

private:
	void OnResize();

	void BuildDescriptorHeap();
	void PostProcess_BuildDescriptorHeap();

	D3D12_RESOURCE_STATES TranslateResourceState(ERHIResourceState InState);

	//for graphics
	int ClientWidth = 800;
	int ClientHeight = 600;

	ComPtr<IDXGIFactory4> Factory;
	ComPtr<IDXGISwapChain> SwapChain;

	ComPtr<ID3D12CommandAllocator> CommandAllocator;
	ComPtr<ID3D12CommandQueue> CommandQueue;
	ComPtr<ID3D12GraphicsCommandList>	CommandList;
	ComPtr<ID3D12Fence>	Fence;
	UINT64 CurrentFenceValue;


	static const int SwapchainBackbufferCount = 2;
	ComPtr<ID3D12Resource>	BackBuffer[SwapchainBackbufferCount];
	ComPtr<ID3D12Resource> DepthStencilBuffer;
	int CurrentBackBuffer = 0;

	DXGI_FORMAT BackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT DepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	D3D12_VIEWPORT ScreenViewport;
	D3D12_RECT ScissorRect;


	//todo: extract to descriptor heap manager
	ComPtr<ID3D12DescriptorHeap>	RtvHeap;
	ComPtr<ID3D12DescriptorHeap>	DsvHeap;

	//heap for cbv srv uav heap
	ComPtr<ID3D12DescriptorHeap> CbvSrvUavHeap;
	//heap for postprocess
	ComPtr<ID3D12DescriptorHeap> Present_CbvSrvUavHeap;

	UINT RtvDescriptorSize;
	UINT DsvDescriptorSize;
	UINT CbvSrvUavDescriptorSize;


	//record allocated Descriptor
	UINT RtvDHAllocatedCount;
	UINT DsvDHAllocatedCount;
	UINT CbvDHAllocatedCount;
	UINT Present_CbvDHAllocatedCount;

	//shader resource
	FRHIShaderResource_D3D12* ShaderResource;
};