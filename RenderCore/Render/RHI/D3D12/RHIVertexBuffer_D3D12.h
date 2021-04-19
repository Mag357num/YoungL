#pragma once
#include "../RHIContext.h"

class FRHIVertexBuffer_D3D12 : public IRHIVertexBuffer
{
public:
	FRHIVertexBuffer_D3D12(){}
	virtual ~FRHIVertexBuffer_D3D12(){
		if (VertexBuffer!= nullptr)
		{
			VertexBuffer.Reset();
		}
		
	}

	D3D12_VERTEX_BUFFER_VIEW GetVBView(){
		D3D12_VERTEX_BUFFER_VIEW Desc;
		Desc.BufferLocation = VertexBuffer->GetGPUVirtualAddress();
		Desc.SizeInBytes = (UINT)VertexBufferSize;
		Desc.StrideInBytes = sizeof(FVertex);

		return Desc;
	}

	void SetVertexBufferSize(size_t InSize){VertexBufferSize = InSize;}

	ComPtr<ID3D12Resource> VertexBuffer;
private:
	size_t VertexBufferSize = 0;
	
};

