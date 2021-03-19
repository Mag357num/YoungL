#pragma once
#include "../../pch.h"
#include "GPUResource.h"

class EsramAllocator;

class PixelBuffer : GPUResource
{
public:
	PixelBuffer():Y_Width(0),Y_Height(0), Y_ArraySize(0), Y_Format(DXGI_FORMAT_UNKNOWN), Y_BankRotation(0) {}
	uint32_t GetWidth(void) const { return Y_Width; }
	uint32_t GetHeight(void) const { return Y_Height; }
	uint32_t GetDepth(void) const { return Y_ArraySize; }

	const DXGI_FORMAT& GetFormat(void)const { return Y_Format; }

	void SetBankRotation(uint32_t RotationAmount) { Y_BankRotation = RotationAmount; }

	//write raw pixel buffer contentes into a file
	void ExportToFile(const std::wstring& FilePath);
protected:

	D3D12_RESOURCE_DESC DescribleTexture2D(uint32_t Width, uint32_t Height, uint32_t DepthOrArraySize, uint32_t NumMips, DXGI_FORMAT ForMat, UINT Flags);
	void AssociateWithResource(ID3D12Device* Device, const std::wstring& Name, ID3D12Resource* Resource, D3D12_RESOURCE_STATES CurrentState);

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

	uint32_t Y_Width;
	uint32_t Y_Height;
	uint32_t Y_ArraySize;

	DXGI_FORMAT Y_Format;
	uint32_t Y_BankRotation;

};
