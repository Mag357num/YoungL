#include "pch.h"

#include "RHIRenderingItem_D3D12.h"

#include "RHIIndexBuffer_D3D12.h"
#include "RHIVertexBuffer_D3D12.h"
#include "RHIConstantBuffer_D3D12.h"
#include "RHIContext_D3D12.h"

namespace D3D12RHI
{
	extern ComPtr<ID3D12Device> M_Device;
}

void FRHIRenderingMesh_D3D12::BuildConstantBuffer(FObjectConstants* InObjConstants, IRHIContext* Context)
{
	FRHIContext_D3D12* RHIContext_D3D12 = reinterpret_cast<FRHIContext_D3D12*>(Context);
	ConstantBuffer = new FRHIConstantBuffer_D3D12<FObjectConstants>();
	FRHIConstantBuffer_D3D12<FObjectConstants>* Buffer = reinterpret_cast<FRHIConstantBuffer_D3D12<FObjectConstants>*>(ConstantBuffer);

	Buffer->UploadBuffer = std::make_unique<FRHIUploadBuffer_D3D12<FObjectConstants>>(new FRHIUploadBuffer_D3D12<FObjectConstants>(true));
	Buffer->UploadBuffer->CreateUploadResource(1);

	Buffer->CopyData(0, *InObjConstants);

	UINT ObjectBufferSize = FRHIUploadBuffer_D3D12<FObjectConstants>::CalcConstantBufferByteSize(sizeof(FObjectConstants));

	FRHIResource_D3D12* UploadResource_D3D12 = reinterpret_cast<FRHIResource_D3D12*>(Buffer->UploadBuffer->GetResource());
	D3D12_GPU_VIRTUAL_ADDRESS GpuAddress = UploadResource_D3D12->Resource->GetGPUVirtualAddress();
	int BufIndex = 0;
	GpuAddress += BufIndex * ObjectBufferSize;

	Buffer->SetRootParameterIndex(0);
	Buffer->SetGpuVirtualAddress(GpuAddress);
}

void FRHIRenderingMesh_D3D12::BuildSkinnedBoneTransBuffer(FBoneTransforms* InTransforms, IRHIContext* Context)
{
	IsSkined = true;
	FRHIContext_D3D12* RHIContext_D3D12 = reinterpret_cast<FRHIContext_D3D12*>(Context);

	BoneTransformsBuffer = new FRHIConstantBuffer_D3D12<FBoneTransforms>();
	FRHIConstantBuffer_D3D12<FBoneTransforms>* Buffer = reinterpret_cast<FRHIConstantBuffer_D3D12<FBoneTransforms>*>(BoneTransformsBuffer);

	Buffer->UploadBuffer = std::make_unique<FRHIUploadBuffer_D3D12<FBoneTransforms>>(new FRHIUploadBuffer_D3D12<FBoneTransforms>(true));
	Buffer->UploadBuffer->CreateUploadResource(1);

	Buffer->CopyData(0, *InTransforms);

	UINT ObjectBufferSize = FRHIUploadBuffer_D3D12<FObjectConstants>::CalcConstantBufferByteSize(sizeof(FBoneTransforms));

	FRHIResource_D3D12* UploadResource_D3D12 = reinterpret_cast<FRHIResource_D3D12*>(Buffer->UploadBuffer->GetResource());
	D3D12_GPU_VIRTUAL_ADDRESS GpuAddress = UploadResource_D3D12->Resource->GetGPUVirtualAddress();
	int BufIndex = 0;
	GpuAddress += BufIndex * ObjectBufferSize;

	Buffer->SetRootParameterIndex(3);////0 for Object constants ; 1: for main pass constants; 2 :for descriptor table
	Buffer->SetGpuVirtualAddress(GpuAddress);
}

void FRHIRenderingMesh_D3D12::BuildIndexBuffer(std::vector<uint16_t>& InIndices)
{
	IndexBuffer = new FRHIIndexBuffer_D3D12();
	FRHIIndexBuffer_D3D12* Buffer = reinterpret_cast<FRHIIndexBuffer_D3D12*>(IndexBuffer);

	//create index buffer view
	IndexBufferSize = sizeof(uint16_t) * InIndices.size();
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

void FRHIRenderingMesh_D3D12::BuildVertexBuffer(std::vector<FVertex>& InVertices)
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
	Buffer->SetStrideInBytes(VertexStrideSize);
}

void FRHIRenderingMesh_D3D12::BuildVertexBuffer(std::vector<FSkinVertex>& InVertices)
{
	VertexBuffer = new FRHIVertexBuffer_D3D12();
	FRHIVertexBuffer_D3D12* Buffer = reinterpret_cast<FRHIVertexBuffer_D3D12*>(VertexBuffer);

	VertexBufferSize = sizeof(FSkinVertex) * InVertices.size();
	VertexStrideSize = sizeof(FSkinVertex);
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
	Buffer->SetStrideInBytes(VertexStrideSize);
}
