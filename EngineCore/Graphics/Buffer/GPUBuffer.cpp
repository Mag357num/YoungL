#include "../../pch.h"
#include "GPUBuffer.h"
#include "../GraphicsCore.h"
#include "../RHI/CommandContext.h"

using namespace Graphics;

D3D12_RESOURCE_DESC GpuBuffer::DescribleBuffer()
{
	D3D12_RESOURCE_DESC Desc = {};
	Desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	Desc.Alignment = 0;
	Desc.DepthOrArraySize = 1;
	Desc.Flags = m_ResourceFlags;
	Desc.Format = DXGI_FORMAT_UNKNOWN;
	Desc.Width = (UINT64)m_BufferSize;
	Desc.Height = 1;
	Desc.MipLevels = 1;
	Desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	Desc.SampleDesc.Count = 1;
	Desc.SampleDesc.Quality = 0;


	return Desc;
}

void GpuBuffer::Create(const std::wstring& name, uint32_t NumElements, uint32_t ElementSize, EsramAllocator& Allocator, const void* initialData /* = nullptr */)
{
	(void)Allocator;
	Create(name, NumElements, ElementSize, initialData);

}

void GpuBuffer::Create(const std::wstring& name, uint32_t NumElements, uint32_t ElementSize, const UploadBuffer& SrcData, uint32_t SrcOffset /* = 0 */)
{
	Destroy();
	m_ElementCount = NumElements;
	m_ElementSize = ElementSize;
	m_BufferSize = NumElements * ElementSize;

	D3D12_RESOURCE_DESC RDesc = DescribleBuffer();

	m_UsageState = D3D12_RESOURCE_STATE_COMMON;

	D3D12_HEAP_PROPERTIES HeapPro;
	HeapPro.Type = D3D12_HEAP_TYPE_DEFAULT;
	HeapPro.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	HeapPro.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	HeapPro.CreationNodeMask = 1;
	HeapPro.VisibleNodeMask = 1;


	ASSERT_SUCCEEDED(g_Device->CreateCommittedResource(&HeapPro, D3D12_HEAP_FLAG_NONE, &RDesc, m_UsageState, nullptr, MY_IID_PPV_ARGS(&m_Resource)));

	m_GpuVirtualAddress = m_Resource->GetGPUVirtualAddress();

	//Command
	CommandContext::InitializeBuffer(*this, SrcData, SrcOffset);

#ifdef RELEASE
	(name)
#else
	m_Resource->SetName(name.c_str());
#endif // DEBUG

	CreateDerivedViews();
}

void GpuBuffer::Create(const std::wstring& name, uint32_t NumElements, uint32_t ElementSize, const void* initialData /* = nullptr */)
{
	Destroy();
	m_ElementCount = NumElements;
	m_ElementSize = ElementSize;
	m_BufferSize = NumElements * ElementSize;

	D3D12_RESOURCE_DESC RDesc = DescribleBuffer();

	m_UsageState = D3D12_RESOURCE_STATE_COMMON;

	D3D12_HEAP_PROPERTIES HeapPro;
	HeapPro.Type = D3D12_HEAP_TYPE_DEFAULT;
	HeapPro.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	HeapPro.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	HeapPro.CreationNodeMask = 1;
	HeapPro.VisibleNodeMask = 1;


	ASSERT_SUCCEEDED(g_Device->CreateCommittedResource(&HeapPro, D3D12_HEAP_FLAG_NONE, &RDesc, m_UsageState, nullptr, MY_IID_PPV_ARGS(&m_Resource)));

	m_GpuVirtualAddress = m_Resource->GetGPUVirtualAddress();

	if (initialData)
	{
		//Command
		CommandContext::InitializeBuffer(*this, initialData, m_BufferSize);
	}

#ifdef RELEASE
	(name)
#else
	m_Resource->SetName(name.c_str());
#endif // DEBUG

	CreateDerivedViews();
}

void GpuBuffer::CreatePlaced(const std::wstring& name, ID3D12Heap* pBackingHeap, uint32_t HeapOffset, uint32_t NumElements, uint32_t ElementSize, const void* initialData /* = nullptr */)
{
	Destroy();
	m_ElementCount = NumElements;
	m_ElementSize = ElementSize;
	m_BufferSize = NumElements * ElementSize;

	D3D12_RESOURCE_DESC RDesc = DescribleBuffer();

	m_UsageState = D3D12_RESOURCE_STATE_COMMON;

	ASSERT_SUCCEEDED(g_Device->CreateCommittedResource(pBackingHeap, D3D12_HEAP_FLAG_NONE, &RDesc, m_UsageState, nullptr, MY_IID_PPV_ARGS(&m_Resource)));

	m_GpuVirtualAddress = m_Resource->GetGPUVirtualAddress();

	if (initialData)
	{
		//Command
		CommandContext::InitializeBuffer(*this, initialData, m_BufferSize);
	}

#ifdef RELEASE
	(name)
#else
	m_Resource->SetName(name.c_str());
#endif // DEBUG

	CreateDerivedViews();
}

void ByteAddressBuffer::CreateDerivedViews()
{
	D3D12_SHADER_RESOURCE_VIEW_DESC SrvDesc;
	SrvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	SrvDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	SrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	SrvDesc.Buffer.NumElements = (UINT)m_BufferSize / 4;
	SrvDesc.Buffer.StructureByteStride = m_ElementSize;
	SrvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;

	if (m_SRV.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
	{
		m_SRV = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}

	g_Device->CreateShaderResourceView(m_Resource.Get(), &SrvDesc, m_SRV);

	D3D12_UNORDERED_ACCESS_VIEW_DESC UavDesc;
	UavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	UavDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	UavDesc.Buffer.NumElements = m_ElementCount;
	UavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;

	if (m_UAV.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
	{
		m_UAV = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}

	g_Device->CreateUnorderedAccessView(m_Resource.Get(), nullptr, &UavDesc, m_UAV);
}

void StructuredBuffer::CreateDerivedViews()
{
	D3D12_SHADER_RESOURCE_VIEW_DESC SrvDesc;
	SrvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	SrvDesc.Format = DXGI_FORMAT_UNKNOWN;
	SrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	SrvDesc.Buffer.NumElements = m_ElementCount;
	SrvDesc.Buffer.StructureByteStride = m_ElementSize;
	SrvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

	if (m_SRV.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
	{
		m_SRV = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}

	g_Device->CreateShaderResourceView(m_Resource.Get(), &SrvDesc, m_SRV);

	D3D12_UNORDERED_ACCESS_VIEW_DESC UavDesc;
	UavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	UavDesc.Format = DXGI_FORMAT_UNKNOWN;
	UavDesc.Buffer.CounterOffsetInBytes = 0;
	UavDesc.Buffer.NumElements = m_ElementCount;
	UavDesc.Buffer.StructureByteStride = m_ElementSize;
	UavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

	if (m_UAV.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
	{
		m_UAV = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}

	m_CounterBuffer.Create(L"StructuredBuffer:Counter", 1,  4);

	g_Device->CreateUnorderedAccessView(m_Resource.Get(), nullptr, &UavDesc, m_UAV);
}

const D3D12_CPU_DESCRIPTOR_HANDLE& StructuredBuffer::GetCounterSRV(CommandContext& Context)
{
	Context.TransitionResource(m_CounterBuffer, D3D12_RESOURCE_STATE_GENERIC_READ);
	return m_CounterBuffer.GetSRV();
}

const D3D12_CPU_DESCRIPTOR_HANDLE& StructuredBuffer::GetCounterUAV(CommandContext& Context)
{
	Context.TransitionResource(m_CounterBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	return m_CounterBuffer.GetUAV();
}

void TypedBuffer::CreateDerivedViews()
{
	D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
	SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	SRVDesc.Format = m_DataFormat;
	SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	SRVDesc.Buffer.NumElements = m_ElementCount;
	SRVDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

	if (m_SRV.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
		m_SRV = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	g_Device->CreateShaderResourceView(m_Resource.Get(), &SRVDesc, m_SRV);

	D3D12_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {};
	UAVDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	UAVDesc.Format = m_DataFormat;
	UAVDesc.Buffer.NumElements = m_ElementCount;
	UAVDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

	if (m_UAV.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
		m_UAV = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	g_Device->CreateUnorderedAccessView(m_Resource.Get(), nullptr, &UAVDesc, m_UAV);
}