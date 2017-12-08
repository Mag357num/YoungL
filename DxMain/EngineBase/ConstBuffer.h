#pragma once

#include "DXSample.h"
#include "LearnHelloWindow.h"
#include <vector>

class ConstBufferSample : public LearnHelloWindow
{
public:
	ConstBufferSample(UINT width, UINT height, std::wstring name);
	~ConstBufferSample();

	virtual void LoadAssets() override;

	virtual void CreateDescriptorHeaps() override;

	virtual void PopulateCommandList() override;

	virtual void OnUpdate() override;

protected:
private:

	struct Vertex
	{
		XMFLOAT3 position;
		XMFLOAT4 color;
	};

	struct SceneConstantBuffer
	{
		XMFLOAT4 offset;
	};

	ComPtr<ID3D12Resource> m_constantBuffer;
	SceneConstantBuffer m_constantBufferData;
	UINT8* m_pCbvDataBegin;

	ComPtr<ID3D12DescriptorHeap> m_cbvHeap;
};