#pragma once
#include "../../pch.h"
#include "GPUResource.h"

class EsramAllocator;

class FPixelBuffer : public FGPUResource
{
public:
	FPixelBuffer():Width(0),Height(0), ArraySize(0), Format(DXGI_FORMAT_UNKNOWN), BankRotation(0) {}
	uint32_t GetWidth(void) const { return Width; }
	uint32_t GetHeight(void) const { return Height; }
	uint32_t GetDepth(void) const { return ArraySize; }

	const DXGI_FORMAT& GetFormat(void)const { return Format; }

	void SetBankRotation(uint32_t RotationAmount) { BankRotation = RotationAmount; }

	//write raw pixel buffer contentes into a file
	void ExportToFile(const std::wstring& FilePath);
protected:

	D3D12_RESOURCE_DESC DescribleTexture2D(uint32_t InWidth, uint32_t InHeight, uint32_t DepthOrArraySize, uint32_t InNumMips, DXGI_FORMAT InForMat, UINT Flags);
	void AssociateWithResource(ID3D12Device* Device, const std::wstring& Name, ID3D12Resource* InResource, D3D12_RESOURCE_STATES CurrentState);

	void CreateTextureResource(ID3D12Device* Device, const std::wstring& Name, const D3D12_RESOURCE_DESC& ResourceDesc, 
		D3D12_CLEAR_VALUE ClearValue, D3D12_GPU_VIRTUAL_ADDRESS VidMemPtr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN);

	void CreateTextureResource(ID3D12Device* Device, const std::wstring& Name, const D3D12_RESOURCE_DESC& ResourceDesc,
		D3D12_CLEAR_VALUE ClearValue, EsramAllocator& Allocator);

	static DXGI_FORMAT GetBaseFormat(DXGI_FORMAT Format);
	static DXGI_FORMAT GetUAVFormat(DXGI_FORMAT Format);
	static DXGI_FORMAT GetDSVFormat(DXGI_FORMAT Format);
	static DXGI_FORMAT GetDepthFormat(DXGI_FORMAT Format);
	static DXGI_FORMAT GetStencilFormat(DXGI_FORMAT Format);
	static size_t BytesPerPixel(DXGI_FORMAT Format);

	uint32_t Width;
	uint32_t Height;
	uint32_t ArraySize;

	DXGI_FORMAT Format;
	uint32_t BankRotation;

};
