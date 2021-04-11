#pragma once
#include "../RHIContext.h"
#include "../../Utilities.h"

#include "RHIIndexBuffer_D3D12.h"
#include "RHIVertexBuffer_D3D12.h"
#include "RHIConstantBuffer_D3D12.h"

namespace D3D12RHI
{
	extern ComPtr<ID3D12Device> M_Device;
}

class FRHIRenderingItem_D3D12 : public IRHIRenderingItem
{
public:
	FRHIRenderingItem_D3D12(){}
	virtual ~FRHIRenderingItem_D3D12()
	{

	}


	virtual void BuildConstantBuffer(FObjectConstants* InObjConstants, IRHIContext* Context)override;
	virtual void BuildIndexBuffer(std::vector<uint32_t>& InIndices)override;
	virtual void BuildVertexBuffer(std::vector<FVertex>& InVertices)override;
private:

};

void FRHIRenderingItem_D3D12::BuildConstantBuffer(FObjectConstants* InObjConstants, IRHIContext* Context)
{
	FRHIContext_D3D12* RHIContext_D3D12 = reinterpret_cast<FRHIContext_D3D12*>(Context);
	ConstantBuffer = new FRHIConstantBuffer_D3D12<FObjectConstants>();
	FRHIConstantBuffer_D3D12<FObjectConstants>* Buffer = reinterpret_cast<FRHIConstantBuffer_D3D12<FObjectConstants>*>(ConstantBuffer);

	Buffer->UploadBuffer = std::make_unique<FRHIUploadBuffer_D3D12<FObjectConstants>>(new FRHIUploadBuffer_D3D12<FObjectConstants>(true));
	Buffer->UploadBuffer->CreateUploadResource(1);
	//mapdata
	//Buffer->UploadBuffer->CopyData(0, *InObjConstants);

	UINT ObjectBufferSize = FRHIUploadBuffer_D3D12<FObjectConstants>::CalcConstantBufferByteSize(sizeof(FObjectConstants));

	D3D12_CPU_DESCRIPTOR_HANDLE CpuDescriptor = RHIContext_D3D12->GetCbvSrvUavDescriptorHeap()->GetCPUDescriptorHandleForHeapStart();
	D3D12_CONSTANT_BUFFER_VIEW_DESC ViewDesc;

	FRHIResource_D3D12* UploadResource_D3D12 = reinterpret_cast<FRHIResource_D3D12*>(Buffer->UploadBuffer->GetResource());
	D3D12_GPU_VIRTUAL_ADDRESS GpuAddress = UploadResource_D3D12->Resource->GetGPUVirtualAddress();
	int BufIndex = 0;
	GpuAddress += BufIndex * ObjectBufferSize;
	ViewDesc.BufferLocation = GpuAddress;
	ViewDesc.SizeInBytes = ObjectBufferSize;

	D3D12RHI::M_Device->CreateConstantBufferView(&ViewDesc, CpuDescriptor);
	Buffer->SetRootParameterIndex(0);
	Buffer->SetGpuhandle(RHIContext_D3D12->GetCbvSrvUavDescriptorHeap()->GetGPUDescriptorHandleForHeapStart());
}

void FRHIRenderingItem_D3D12::BuildIndexBuffer(std::vector<uint32_t>& InIndices)
{
	IndexBuffer = new FRHIIndexBuffer_D3D12();
	FRHIIndexBuffer_D3D12* Buffer = reinterpret_cast<FRHIIndexBuffer_D3D12*>(IndexBuffer);
	
	//create index buffer view
	IndexBufferSize = sizeof(uint32_t) * InIndices.size();
	CD3DX12_RESOURCE_DESC IxResDesc = CD3DX12_RESOURCE_DESC::Buffer(IndexBufferSize);
	CD3DX12_HEAP_PROPERTIES HeapProperties(D3D12_HEAP_TYPE_UPLOAD);
	D3D12RHI::M_Device->CreateCommittedResource(&HeapProperties,
		D3D12_HEAP_FLAG_NONE, &IxResDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
		IID_PPV_ARGS(&Buffer->IndexBuffer));

	UINT8* IndexBegin;
	D3D12_RANGE IdCopyRange;
	Buffer->IndexBuffer->Map(0, &IdCopyRange, reinterpret_cast<void**>(&IndexBegin));
	memcpy(IndexBegin, InIndices.data(), IndexBufferSize);
	Buffer->IndexBuffer->Unmap(0, nullptr);

	IndexCount = InIndices.size();
	Buffer->SetIndexBufferSize(IndexBufferSize);
}

void FRHIRenderingItem_D3D12::BuildVertexBuffer(std::vector<FVertex>& InVertices)
{
	VertexBuffer = new FRHIVertexBuffer_D3D12();
	FRHIVertexBuffer_D3D12* Buffer = reinterpret_cast<FRHIVertexBuffer_D3D12*>(VertexBuffer);

	VertexBufferSize = sizeof(FVertex) * InVertices.size();
	VertexStrideSize = sizeof(FVertex);
	CD3DX12_HEAP_PROPERTIES HeapProperties(D3D12_HEAP_TYPE_UPLOAD);
	CD3DX12_RESOURCE_DESC VtResDesc = CD3DX12_RESOURCE_DESC::Buffer(VertexBufferSize);
	D3D12RHI::M_Device->CreateCommittedResource(&HeapProperties,
		D3D12_HEAP_FLAG_NONE, &VtResDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
		IID_PPV_ARGS(&Buffer->VertexBuffer));

	UINT8* VertexBegin;
	D3D12_RANGE VtCopyRange;
	Buffer->VertexBuffer->Map(0, &VtCopyRange, reinterpret_cast<void**>(&VertexBegin));
	memcpy(VertexBegin, InVertices.data(), VertexBufferSize);
	Buffer->VertexBuffer->Unmap(0, nullptr);

	Buffer->SetVertexBufferSize(VertexBufferSize);
}

