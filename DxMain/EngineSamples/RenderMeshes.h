#pragma once

#include "DXSample.h"

using namespace DirectX;
using namespace Microsoft::WRL;

class FRenderMeshes : public DXSample
{
public:
	FRenderMeshes(UINT Width, UINT Height, std::wstring name);

	virtual void OnInit() override;
	virtual void OnUpdate() override;
	virtual void OnRender() override;
	virtual void OnDestroy() override;

	static const UINT FrameCount = 2;

private:
	void LoadPipeline();
	void LoadAsset();

	void PopulateCommandList();
	void WaitForPreviousFrame();

private:
	ComPtr<IDXGIFactory4> DXFactory;
	ComPtr<ID3D12Device> m_Device;
	ComPtr<ID3D12CommandQueue> m_CommandQueue;
	ComPtr<IDXGISwapChain3> m_SwapChain;
	ComPtr<ID3D12DescriptorHeap> m_DescriptorHeap;
	ComPtr<ID3D12CommandAllocator> m_CommandAllocator;
	ComPtr<ID3D12GraphicsCommandList> m_CommandList;

	UINT m_FrameIndex;
	UINT m_RtvDescriptorSize;

	ComPtr<ID3D12Fence1> m_Fence;
	HANDLE m_FenceEvent;

	ComPtr<ID3D12Resource> m_RenderTarget[FrameCount];

	UINT64 m_FenceValue;
};