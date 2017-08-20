#pragma once

#include "stdafx.h"

using namespace Microsoft::WRL;

class DXBase
{
public:
	DXBase();
	DXBase(UINT inWidth, UINT inHeight, std::wstring InName);
	~DXBase();

	UINT getWidth();
	UINT getHeight();
	std::wstring getName();

	void setWND(HWND* inWND);

	void ThrowIfFailed(HRESULT hr)
	{
		if (FAILED(hr))
		{
			throw std::exception();
		}
	}

	void initPipeline();

	void onUpdate();

	void onRender();

	void onDestroy();

protected:
	void LoadPipeLine();
	void LoadAssets();

	void GetHardWareAdapter(IDXGIFactory4* pFactory, IDXGIAdapter1** ppAdapter);

	void waitForPreviousFrame();

	void populateCommandList();

private:
	UINT width;
	UINT height;
	std::wstring WindowName;

	HWND savedWND;

	ComPtr<IDXGIFactory4> factory;
	ComPtr<IDXGIAdapter1> hardwareAdapter;
	ComPtr<ID3D12Device1> m_Device;
	ComPtr<ID3D12CommandQueue> m_commandQueue;
	ComPtr<IDXGISwapChain3> m_swapChain;
	ComPtr<ID3D12DescriptorHeap> m_heapDesc;
	UINT m_rtvDescriptorSize;

	static const UINT FrameCount = 2;

	ComPtr<ID3D12Resource> m_renderTargets[FrameCount];

	ComPtr<ID3D12CommandAllocator> m_commandAllocator;
	UINT m_curentFrameIndex;

	ComPtr<ID3D12GraphicsCommandList> m_commandList;

	ComPtr<ID3D12PipelineState> m_pipelineState;

	//synchron
	ComPtr<ID3D12Fence> m_fence;
	UINT m_fenceValue;
	HANDLE m_fenceEvent;

};