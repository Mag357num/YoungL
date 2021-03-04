#pragma once

#include "GPUResource.h"

class EsramAllocator;

class GpuBuffer : public GPUResource
{
public:
	~GpuBuffer() { Destroy(); }

	void Create(const std::wstring& name, uint32_t NumElements, uint32_t ElementSize, const void* initialData = nullptr);

	void Create(const std::wstring& name, uint32_t NumElements, uint32_t ElementSize, EsrmAllocator& Allocator,
		const void* initialData = nullptr);

	void CreatePlaced(const std::wstring& name, ID3D12Heap* pBackingHeap, uint32_t HeapOffset, uint32_t NumElements,
		uint32_t ElementSize, const void* initialData = nullptr);

	const D3D12_CPU_DESCRIPTOR_HANDLE& GetUAV(void) const { return m_UAV; }
	const D3D12_CPU_DESCRIPTOR_HANDLE& GetSRV(void) const { return m_SRV; }

	D3D12_GPU_VIRTUAL_ADDRESS RootConstantBufferView() const { return m_GpuVirtualAddress; }

	D3D12_CPU_DESCRIPTOR_HANDLE CreateConstantBufferView(uint32_t Offset, uint32_t Size) const;

	D3D12_VERTEX_BUFFER_VIEW VertexBufferView(size_t Offset, uint32_t Size, uint32_t Stride) const;
	D3D12_VERTEX_BUFFER_VIEW VertexBufferView(size_t BaseVertexIndex = 0) const
	{
		uint32_t Offset = BaseVertexIndex * m_ElementSize;
		return VertexBufferView(Offset, (uint32_t)(m_BufferSize - Offset), m_ElementSize);
	}

	D3D12_INDEX_BUFFER_VIEW IndexBufferView(size_t Offset, uint32_t Size, bool b32Bit = false) const;
	D3D12_INDEX_BUFFER_VIEW IndexBufferView(size_t StartIndex = 0) const
	{
		size_t Offset = StartIndex * m_ElementSize;
		return IndexBufferView(Offset, (uint32_t)(m_BufferSize - Offset), m_ElementSize ==4);
	}

	size_t GetBufferSize() { return m_BufferSize; }
	uint32_t GetElementCount() { return m_ElementCount; }
	uint32_t GetElementSize() { return m_ElementSize; }

protected:

	GpuBuffer(void) :
		m_BufferSize(0),
		m_ElementCount(0),
		m_ElementSize(0)
	{
		m_ResourceFlags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		m_UAV.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
		m_SRV.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
	}

	D3D12_RESOURCE_DESC DescribleBuffer(void);
	virtual void CreateDerivedViews(void) = 0;

	D3D12_CPU_DESCRIPTOR_HANDLE m_UAV;
	D3D12_CPU_DESCRIPTOR_HANDLE m_SRV;

	size_t m_BufferSize;
	uint32_t m_ElementCount;
	uint32_t m_ElementSize;

	D3D12_RESOURCE_FLAGS m_ResourceFlags;

};


class ByteAddressBuffer : public GpuBuffer
{
public:
	virtual void CreateDerivedViews(void) override;
};

class IndirectArgsBuffer : public GpuBuffer
{
public:
	IndirectArgsBuffer(void)
	{

	}
};

class StructuredBuffer : public GpuBuffer
{
public:
	virtual void Destroy()override
	{
		m_CounterBuffer.Destroy();
		GpuBuffer::Destroy();
	}

	virtual void CreateDerivedViews(void)override;

	ByteAddressBuffer& GetCounterBuffer(void) { return m_CounterBuffer; }

	const D3D12_CPU_DESCRIPTOR_HANDLE& GetCounterUAV(CommandContext& Context);
	const D3D12_CPU_DESCRIPTOR_HANDLE& GetCounterSRV(CommandContext& Context);

private:
	ByteAddressBuffer m_CounterBuffer;
};

class TypedBuffer : public GpuBuffer
{
public:
	TypedBuffer(DXGI_FORMAT Format) : m_DataFormat(Format)
	{

	}


	virtual void CreateDerivedViews(void)override;

private:
	DXGI_FORMAT m_DataFormat;
};

