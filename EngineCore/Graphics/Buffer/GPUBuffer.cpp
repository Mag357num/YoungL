#include "../../pch.h"
#include "GPUBuffer.h"
#include "../GraphicsCore.h"
#include "../RHI/CommandContext.h"

#include "../../Math/Common.h"

using namespace Graphics;

D3D12_RESOURCE_DESC GpuBuffer::DescribleBuffer()
{
	D3D12_RESOURCE_DESC Desc = {};
	Desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	Desc.Alignment = 0;
	Desc.DepthOrArraySize = 1;
	Desc.Flags = Y_ResourceFlags;
	Desc.Format = DXGI_FORMAT_UNKNOWN;
	Desc.Width = (UINT64)Y_BufferSize;
	Desc.Height = 1;
	Desc.MipLevels = 1;
	Desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	Desc.SampleDesc.Count = 1;
	Desc.SampleDesc.Quality = 0;


	return Desc;
}

D3D12_CPU_DESCRIPTOR_HANDLE GpuBuffer::CreateConstantBufferView(uint32_t Offset, uint32_t Size) const
{
	Size = Math::AlignUp(Size, 16);

	D3D12_CONSTANT_BUFFER_VIEW_DESC CbvDesc;
	CbvDesc.BufferLocation = Y_GpuVirtualAddress + Offset;
	CbvDesc.SizeInBytes = Size;

	D3D12_CPU_DESCRIPTOR_HANDLE hCbv = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	g_Device->CreateConstantBufferView(&CbvDesc, hCbv);

	return hCbv;
}


void GpuBuffer::Create(const std::wstring& name, uint32_t NumElements, uint32_t ElementSize, EsramAllocator& Allocator, const void* initialData /* = nullptr */)
{
	(void)Allocator;
	Create(name, NumElements, ElementSize, initialData);

}

void GpuBuffer::Create(const std::wstring& name, uint32_t NumElements, uint32_t ElementSize, const UploadBuffer& SrcData, uint32_t SrcOffset /* = 0 */)
{
	Destroy();
	Y_ElementCount = NumElements;
	Y_ElementSize = ElementSize;
	Y_BufferSize = NumElements * ElementSize;

	D3D12_RESOURCE_DESC RDesc = DescribleBuffer();

	Y_UsageState = D3D12_RESOURCE_STATE_COMMON;

	D3D12_HEAP_PROPERTIES HeapPro;
	HeapPro.Type = D3D12_HEAP_TYPE_DEFAULT;
	HeapPro.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	HeapPro.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	HeapPro.CreationNodeMask = 1;
	HeapPro.VisibleNodeMask = 1;


	ASSERT_SUCCEEDED(g_Device->CreateCommittedResource(&HeapPro, D3D12_HEAP_FLAG_NONE, &RDesc, Y_UsageState, nullptr, MY_IID_PPV_ARGS(&Y_Resource)));

	Y_GpuVirtualAddress = Y_Resource->GetGPUVirtualAddress();

	//Command
	CommandContext::InitializeBuffer(*this, SrcData, SrcOffset);

#ifdef RELEASE
	(name)
#else
	Y_Resource->SetName(name.c_str());
#endif // DEBUG

	CreateDerivedViews();
}

void GpuBuffer::Create(const std::wstring& name, uint32_t NumElements, uint32_t ElementSize, const void* initialData /* = nullptr */)
{
	Destroy();
	Y_ElementCount = NumElements;
	Y_ElementSize = ElementSize;
	Y_BufferSize = NumElements * ElementSize;

	D3D12_RESOURCE_DESC RDesc = DescribleBuffer();

	Y_UsageState = D3D12_RESOURCE_STATE_COMMON;

	D3D12_HEAP_PROPERTIES HeapPro;
	HeapPro.Type = D3D12_HEAP_TYPE_DEFAULT;
	HeapPro.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	HeapPro.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	HeapPro.CreationNodeMask = 1;
	HeapPro.VisibleNodeMask = 1;


	ASSERT_SUCCEEDED(g_Device->CreateCommittedResource(&HeapPro, D3D12_HEAP_FLAG_NONE, &RDesc, Y_UsageState, nullptr, MY_IID_PPV_ARGS(&Y_Resource)));

	Y_GpuVirtualAddress = Y_Resource->GetGPUVirtualAddress();

	if (initialData)
	{
		//Command
		CommandContext::InitializeBuffer(*this, initialData, Y_BufferSize);
	}

#ifdef RELEASE
	(name)
#else
	Y_Resource->SetName(name.c_str());
#endif // DEBUG

	CreateDerivedViews();
}

void GpuBuffer::CreatePlaced(const std::wstring& name, ID3D12Heap* pBackingHeap, uint32_t HeapOffset, uint32_t NumElements, uint32_t ElementSize, const void* initialData /* = nullptr */)
{
	Destroy();
	Y_ElementCount = NumElements;
	Y_ElementSize = ElementSize;
	Y_BufferSize = NumElements * ElementSize;

	D3D12_RESOURCE_DESC RDesc = DescribleBuffer();

	Y_UsageState = D3D12_RESOURCE_STATE_COMMON;

	ASSERT_SUCCEEDED(g_Device->CreateCommittedResource(pBackingHeap, D3D12_HEAP_FLAG_NONE, &RDesc, Y_UsageState, nullptr, MY_IID_PPV_ARGS(&Y_Resource)));

	Y_GpuVirtualAddress = Y_Resource->GetGPUVirtualAddress();

	if (initialData)
	{
		//Command
		CommandContext::InitializeBuffer(*this, initialData, Y_BufferSize);
	}

#ifdef RELEASE
	(name)
#else
	Y_Resource->SetName(name.c_str());
#endif // DEBUG

	CreateDerivedViews();
}

void ByteAddressBuffer::CreateDerivedViews()
{
	D3D12_SHADER_RESOURCE_VIEW_DESC SrvDesc;
	SrvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	SrvDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	SrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	SrvDesc.Buffer.NumElements = (UINT)Y_BufferSize / 4;
	SrvDesc.Buffer.StructureByteStride = Y_ElementSize;
	SrvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;

	if (Y_SRV.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
	{
		Y_SRV = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}

	g_Device->CreateShaderResourceView(Y_Resource.Get(), &SrvDesc, Y_SRV);

	D3D12_UNORDERED_ACCESS_VIEW_DESC UavDesc;
	UavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	UavDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	UavDesc.Buffer.NumElements = Y_ElementCount;
	UavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;

	if (Y_UAV.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
	{
		Y_UAV = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}

	g_Device->CreateUnorderedAccessView(Y_Resource.Get(), nullptr, &UavDesc, Y_UAV);
}

void StructuredBuffer::CreateDerivedViews()
{
	D3D12_SHADER_RESOURCE_VIEW_DESC SrvDesc;
	SrvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	SrvDesc.Format = DXGI_FORMAT_UNKNOWN;
	SrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	SrvDesc.Buffer.NumElements = Y_ElementCount;
	SrvDesc.Buffer.StructureByteStride = Y_ElementSize;
	SrvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

	if (Y_SRV.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
	{
		Y_SRV = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}

	g_Device->CreateShaderResourceView(Y_Resource.Get(), &SrvDesc, Y_SRV);

	D3D12_UNORDERED_ACCESS_VIEW_DESC UavDesc;
	UavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	UavDesc.Format = DXGI_FORMAT_UNKNOWN;
	UavDesc.Buffer.CounterOffsetInBytes = 0;
	UavDesc.Buffer.NumElements = Y_ElementCount;
	UavDesc.Buffer.StructureByteStride = Y_ElementSize;
	UavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

	if (Y_UAV.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
	{
		Y_UAV = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}

	Y_CounterBuffer.Create(L"StructuredBuffer:Counter", 1,  4);

	g_Device->CreateUnorderedAccessView(Y_Resource.Get(), nullptr, &UavDesc, Y_UAV);
}

const D3D12_CPU_DESCRIPTOR_HANDLE& StructuredBuffer::GetCounterSRV(CommandContext& Context)
{
	Context.TransitionResource(Y_CounterBuffer, D3D12_RESOURCE_STATE_GENERIC_READ);
	return Y_CounterBuffer.GetSRV();
}

const D3D12_CPU_DESCRIPTOR_HANDLE& StructuredBuffer::GetCounterUAV(CommandContext& Context)
{
	Context.TransitionResource(Y_CounterBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	return Y_CounterBuffer.GetUAV();
}

void TypedBuffer::CreateDerivedViews()
{
	D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
	SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	SRVDesc.Format = Y_DataFormat;
	SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	SRVDesc.Buffer.NumElements = Y_ElementCount;
	SRVDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

	if (Y_SRV.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
		Y_SRV = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	g_Device->CreateShaderResourceView(Y_Resource.Get(), &SRVDesc, Y_SRV);

	D3D12_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {};
	UAVDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	UAVDesc.Format = Y_DataFormat;
	UAVDesc.Buffer.NumElements = Y_ElementCount;
	UAVDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

	if (Y_UAV.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
		Y_UAV = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	g_Device->CreateUnorderedAccessView(Y_Resource.Get(), nullptr, &UAVDesc, Y_UAV);
}