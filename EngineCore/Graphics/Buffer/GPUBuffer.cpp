#include "../../pch.h"
#include "GPUBuffer.h"
#include "../GraphicsCore.h"
#include "../RHI/CommandContext.h"

#include "../../Math/Common.h"

using namespace Graphics;

D3D12_RESOURCE_DESC FGpuBuffer::DescribleBuffer()
{
	D3D12_RESOURCE_DESC Desc = {};
	Desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	Desc.Alignment = 0;
	Desc.DepthOrArraySize = 1;
	Desc.Flags = ResourceFlags;
	Desc.Format = DXGI_FORMAT_UNKNOWN;
	Desc.Width = (UINT64)BufferSize;
	Desc.Height = 1;
	Desc.MipLevels = 1;
	Desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	Desc.SampleDesc.Count = 1;
	Desc.SampleDesc.Quality = 0;


	return Desc;
}

D3D12_CPU_DESCRIPTOR_HANDLE FGpuBuffer::CreateConstantBufferView(uint32_t Offset, uint32_t Size) const
{
	Size = Math::AlignUp(Size, 16);

	D3D12_CONSTANT_BUFFER_VIEW_DESC CbvDesc;
	CbvDesc.BufferLocation = GpuVirtualAddress + Offset;
	CbvDesc.SizeInBytes = Size;

	D3D12_CPU_DESCRIPTOR_HANDLE hCbv = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	g_Device->CreateConstantBufferView(&CbvDesc, hCbv);

	return hCbv;
}


void FGpuBuffer::Create(const std::wstring& Name, uint32_t NumElements, uint32_t ElementSize, EsramAllocator& Allocator, const void* InitialData /* = nullptr */)
{
	(void)Allocator;
	Create(Name, NumElements, ElementSize, InitialData);

}

void FGpuBuffer::Create(const std::wstring& Name, uint32_t NumElements, uint32_t ElementSize, const FUploadBuffer& SrcData, uint32_t SrcOffset /* = 0 */)
{
	Destroy();
	ElementCount = NumElements;
	ElementSize = ElementSize;
	BufferSize = NumElements * ElementSize;

	D3D12_RESOURCE_DESC RDesc = DescribleBuffer();

	UsageState = D3D12_RESOURCE_STATE_COMMON;

	D3D12_HEAP_PROPERTIES HeapPro;
	HeapPro.Type = D3D12_HEAP_TYPE_DEFAULT;
	HeapPro.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	HeapPro.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	HeapPro.CreationNodeMask = 1;
	HeapPro.VisibleNodeMask = 1;


	ASSERT_SUCCEEDED(g_Device->CreateCommittedResource(&HeapPro, D3D12_HEAP_FLAG_NONE, &RDesc, UsageState, nullptr, MY_IID_PPV_ARGS(&Resource)));

	GpuVirtualAddress = Resource->GetGPUVirtualAddress();

	//Command
	FCommandContext::InitializeBuffer(*this, SrcData, SrcOffset);

#ifdef RELEASE
	(name)
#else
	Resource->SetName(Name.c_str());
#endif // DEBUG

	CreateDerivedViews();
}

void FGpuBuffer::Create(const std::wstring& Name, uint32_t NumElements, uint32_t ElementSize, const void* InitialData /* = nullptr */)
{
	Destroy();
	ElementCount = NumElements;
	ElementSize = ElementSize;
	BufferSize = NumElements * ElementSize;

	D3D12_RESOURCE_DESC RDesc = DescribleBuffer();

	UsageState = D3D12_RESOURCE_STATE_COMMON;

	D3D12_HEAP_PROPERTIES HeapPro;
	HeapPro.Type = D3D12_HEAP_TYPE_DEFAULT;
	HeapPro.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	HeapPro.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	HeapPro.CreationNodeMask = 1;
	HeapPro.VisibleNodeMask = 1;


	ASSERT_SUCCEEDED(g_Device->CreateCommittedResource(&HeapPro, D3D12_HEAP_FLAG_NONE, &RDesc, UsageState, nullptr, MY_IID_PPV_ARGS(&Resource)));

	GpuVirtualAddress = Resource->GetGPUVirtualAddress();

	if (InitialData)
	{
		//Command
		FCommandContext::InitializeBuffer(*this, InitialData, BufferSize);
	}

#ifdef RELEASE
	(name)
#else
	Resource->SetName(Name.c_str());
#endif // DEBUG

	CreateDerivedViews();
}

void FGpuBuffer::CreatePlaced(const std::wstring& Name, ID3D12Heap* pBackingHeap, uint32_t HeapOffset, uint32_t NumElements, uint32_t ElementSize, const void* InitialData /* = nullptr */)
{
	Destroy();
	ElementCount = NumElements;
	ElementSize = ElementSize;
	BufferSize = NumElements * ElementSize;

	D3D12_RESOURCE_DESC RDesc = DescribleBuffer();

	UsageState = D3D12_RESOURCE_STATE_COMMON;

	ASSERT_SUCCEEDED(g_Device->CreatePlacedResource(pBackingHeap, D3D12_HEAP_FLAG_NONE, &RDesc, UsageState, nullptr, MY_IID_PPV_ARGS(&Resource)));

	GpuVirtualAddress = Resource->GetGPUVirtualAddress();

	if (InitialData)
	{
		//Command
		FCommandContext::InitializeBuffer(*this, InitialData, BufferSize);
	}

#ifdef RELEASE
	(name)
#else
	Resource->SetName(Name.c_str());
#endif // DEBUG

	CreateDerivedViews();
}

void FByteAddressBuffer::CreateDerivedViews()
{
	D3D12_SHADER_RESOURCE_VIEW_DESC SrvDesc;
	SrvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	SrvDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	SrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	SrvDesc.Buffer.NumElements = (UINT)BufferSize / 4;
	SrvDesc.Buffer.StructureByteStride = ElementSize;
	SrvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;

	if (SRV.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
	{
		SRV = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}

	g_Device->CreateShaderResourceView(Resource.Get(), &SrvDesc, SRV);

	D3D12_UNORDERED_ACCESS_VIEW_DESC UavDesc;
	UavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	UavDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	UavDesc.Buffer.NumElements = ElementCount;
	UavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;

	if (UAV.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
	{
		UAV = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}

	g_Device->CreateUnorderedAccessView(Resource.Get(), nullptr, &UavDesc, UAV);
}

void FStructuredBuffer::CreateDerivedViews()
{
	D3D12_SHADER_RESOURCE_VIEW_DESC SrvDesc;
	SrvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	SrvDesc.Format = DXGI_FORMAT_UNKNOWN;
	SrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	SrvDesc.Buffer.NumElements = ElementCount;
	SrvDesc.Buffer.StructureByteStride = ElementSize;
	SrvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

	if (SRV.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
	{
		SRV = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}

	g_Device->CreateShaderResourceView(Resource.Get(), &SrvDesc, SRV);

	D3D12_UNORDERED_ACCESS_VIEW_DESC UavDesc;
	UavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	UavDesc.Format = DXGI_FORMAT_UNKNOWN;
	UavDesc.Buffer.CounterOffsetInBytes = 0;
	UavDesc.Buffer.NumElements = ElementCount;
	UavDesc.Buffer.StructureByteStride = ElementSize;
	UavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

	if (UAV.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
	{
		UAV = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}

	CounterBuffer.Create(L"StructuredBuffer:Counter", 1,  4);

	g_Device->CreateUnorderedAccessView(Resource.Get(), nullptr, &UavDesc, UAV);
}

const D3D12_CPU_DESCRIPTOR_HANDLE& FStructuredBuffer::GetCounterSRV(FCommandContext& Context)
{
	Context.TransitionResource(CounterBuffer, D3D12_RESOURCE_STATE_GENERIC_READ);
	return CounterBuffer.GetSRV();
}

const D3D12_CPU_DESCRIPTOR_HANDLE& FStructuredBuffer::GetCounterUAV(FCommandContext& Context)
{
	Context.TransitionResource(CounterBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	return CounterBuffer.GetUAV();
}

void FTypedBuffer::CreateDerivedViews()
{
	D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
	SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	SRVDesc.Format = DataFormat;
	SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	SRVDesc.Buffer.NumElements = ElementCount;
	SRVDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

	if (SRV.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
		SRV = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	g_Device->CreateShaderResourceView(Resource.Get(), &SRVDesc, SRV);

	D3D12_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {};
	UAVDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	UAVDesc.Format = DataFormat;
	UAVDesc.Buffer.NumElements = ElementCount;
	UAVDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

	if (UAV.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
		UAV = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	g_Device->CreateUnorderedAccessView(Resource.Get(), nullptr, &UAVDesc, UAV);
}