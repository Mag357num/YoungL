#pragma once

#include <wrl.h>
#include <d3d12.h>

using namespace Microsoft::WRL;

template<typename T>
class UploadBuffer
{
public:
	UploadBuffer(ID3D12Device* Device, UINT ElementCount, bool IsConstant)
		:IsConstantBuffer(IsConstant)
	{
		ElementSize = sizeof(T);

		//constant buffer elements need to be multiples of 256 bytes
		// This is because the hardware can only view constant data 
		// at m*256 byte offsets and of n*256 byte lengths. 
		if (IsConstantBuffer)
		{
			ElementSize = UploadBuffer::CalcConstantBufferByteSize(sizeof(T));
		}

		Device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(ElementSize * ElementCount),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&UploadResource)
			);

		UploadResource->Map(0, nullptr, reinterpret_cast<void**>(&MappedData));

	}

	UploadBuffer(const UploadBuffer& rhs) = delete;
	UploadBuffer& operator=(const UploadBuffer& rhs) = delete;

	~UploadBuffer()
	{
		if (UploadResource !=nullptr)
		{
			UploadResource->Unmap(0, nullptr);
		}

		MappedData = nullptr;
	}


	ID3D12Resource* GetResource()
	{
		return UploadResource.Get();
	}

	void CopyData(int ElementIndex, const T& Data)
	{
		memcpy(&MappedData[ElementIndex*ElementSize], &Data, sizeof(T));
	}

	static UINT CalcConstantBufferByteSize(UINT ByteSize)
	{
		// Constant buffers must be a multiple of the minimum hardware
		// allocation size (usually 256 bytes).  So round up to nearest
		// multiple of 256.  We do this by adding 255 and then masking off
		// the lower 2 bytes which store all bits < 256.
		// Example: Suppose byteSize = 300.
		// (300 + 255) & ~255
		// 555 & ~255
		// 0x022B & ~0x00ff
		// 0x022B & 0xff00
		// 0x0200
		// 512
		return (ByteSize + 255) & ~255;
	}

private:

	ComPtr<ID3D12Resource> UploadResource;
	BYTE* MappedData = nullptr;

	UINT ElementSize = 0;
	bool IsConstantBuffer = false;
};

