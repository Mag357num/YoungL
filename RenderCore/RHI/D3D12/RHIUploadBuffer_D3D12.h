#pragma once
#include "../RHIContext.h"
#include "RHIResource_D3D12.h"

namespace D3D12RHI
{
	extern ComPtr<ID3D12Device> M_Device;
}

template<typename T>
class FRHIUploadBuffer_D3D12
{
public:
	FRHIUploadBuffer_D3D12(bool IsConstant)
	:IsConstantBuffer(IsConstant)
	{
	}


	virtual ~FRHIUploadBuffer_D3D12() {
		if (UploadResource != nullptr )
		{
			FRHIResource_D3D12* UploadResource_D3D12 = reinterpret_cast<FRHIResource_D3D12*>(UploadResource);
			if (UploadResource_D3D12->Resource != nullptr)
			{
				UploadResource_D3D12->Resource->Unmap(0, nullptr);
			}

			delete UploadResource;
			UploadResource = nullptr;
		}

		MappedData = nullptr;
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

	virtual void CreateUploadResource(UINT ElementCount);
	
	virtual void CopyData(int ElementIndex, const T& Data);
	IRHIResource* GetResource() { return UploadResource; }
private:
	BYTE* MappedData = nullptr;
	UINT ElementSize = 0;

	IRHIResource* UploadResource;

	bool IsConstantBuffer = false;

};

template<typename T>
void FRHIUploadBuffer_D3D12<T>::CreateUploadResource(UINT ElementCount)
{
	UploadResource = new FRHIResource_D3D12();
	FRHIResource_D3D12* UploadResource_D3D12 = reinterpret_cast<FRHIResource_D3D12*>(UploadResource);
	ElementSize = sizeof(T);

	//constant buffer elements need to be multiples of 256 bytes
	// This is because the hardware can only view constant data 
	// at m*256 byte offsets and of n*256 byte lengths. 
	if (IsConstantBuffer)
	{
		ElementSize = CalcConstantBufferByteSize(sizeof(T));
	}

	CD3DX12_HEAP_PROPERTIES HeapProperty(D3D12_HEAP_TYPE_UPLOAD);
	D3D12_RESOURCE_DESC ResDesc = CD3DX12_RESOURCE_DESC::Buffer(ElementSize * ElementCount);
	D3D12RHI::M_Device->CreateCommittedResource(&HeapProperty,
		D3D12_HEAP_FLAG_NONE,
		&ResDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&UploadResource_D3D12->Resource)
	);

	UploadResource_D3D12->Resource->Map(0, nullptr, reinterpret_cast<void**>(&MappedData));
}

template<typename T>
void FRHIUploadBuffer_D3D12<T>::CopyData(int ElementIndex, const T& Data)
{
	memcpy(&MappedData[ElementIndex * ElementSize], &Data, sizeof(T));
}
