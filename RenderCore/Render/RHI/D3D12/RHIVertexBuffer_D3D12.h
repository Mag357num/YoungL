#pragma once
#include "../RHIVertexBuffer.h"
#include <d3d12.h>

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
		Desc.StrideInBytes = (UINT)StrideInBytes;

		return Desc;
	}

	void SetVertexBufferSize(size_t InSize){VertexBufferSize = InSize;}
	void SetStrideInBytes(size_t InStride) { StrideInBytes = InStride; }

	ComPtr<ID3D12Resource> VertexBuffer;
private:
	size_t VertexBufferSize = 0;
	size_t StrideInBytes =0;
	
};

