#pragma once

#include "../RHIContext.h"

class FRHIIndexBuffer_D3D12 : public IRHIIndexBuffer
{
public:
	FRHIIndexBuffer_D3D12(){}
	virtual ~FRHIIndexBuffer_D3D12(){
		if (IndexBuffer != nullptr)
		{
			IndexBuffer.Reset();
		}
	}

	D3D12_INDEX_BUFFER_VIEW GetIBView(){
		D3D12_INDEX_BUFFER_VIEW Desc;
		Desc.BufferLocation = IndexBuffer->GetGPUVirtualAddress();
		Desc.SizeInBytes = (UINT)IndexBufferSize;
		Desc.Format = DXGI_FORMAT_R32_UINT;

		return Desc;
	}

	void SetIndexBufferSize(size_t InSize){ IndexBufferSize = InSize;}

	ComPtr<ID3D12Resource> IndexBuffer;
private:
	size_t IndexBufferSize;

};
