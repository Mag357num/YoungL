#pragma once

#include "DXSample.h"
#include "LearnHelloWindow.h"
#include <vector>

class FrameBufferSample : public LearnHelloWindow
{
public:
	FrameBufferSample(UINT width, UINT height, std::wstring name);
	~FrameBufferSample();

	virtual void OnRender() override;

	virtual void OnDestroy() override;

protected:
	virtual void LoadAssets() override;

	virtual void PopulateCommandList() override;

	virtual void CreateRTAndCmdAllocator() override;

private:
	
	void WaitForGPU();
	void MoveToNextFrame();

	ComPtr<ID3D12CommandAllocator> m_commandAllocators[FrameCount];

	UINT64 m_fenceValues[FrameCount];
};

