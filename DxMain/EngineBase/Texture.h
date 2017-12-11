#pragma once

#include "DXSample.h"
#include "LearnHelloWindow.h"
#include <vector>

class TextureSample : public LearnHelloWindow
{
public:
	TextureSample(UINT width, UINT height, std::wstring name);
	~TextureSample();

	virtual void LoadAssets() override;

	

protected:

	virtual void PopulateCommandList() override;

	virtual void CreateDescriptorHeaps() override;

	struct Vertex
	{
		XMFLOAT3 position;
		XMFLOAT2 color;
	};
private:

	static const UINT TextureWidth = 256;
	static const UINT TextureHeight = 256;
	static const UINT TexturePixelSize = 4;	// The number of bytes used to represent a pixel in the texture.

	ComPtr<ID3D12Resource> m_texture;

	ComPtr<ID3D12DescriptorHeap> m_srvHeap;

	std::vector<UINT8> GenerateTextureData();
};
