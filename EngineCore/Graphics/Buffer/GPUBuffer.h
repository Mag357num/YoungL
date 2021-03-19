#pragma once

#include "GPUResource.h"
#include "UploadBuffer.h"

class EsramAllocator;

class GpuBuffer : public GPUResource
{
public:
	~GpuBuffer() { Destroy(); }

	void Create(const std::wstring& name, uint32_t NumElements, uint32_t ElementSize, const void* initialData = nullptr);

	void Create(const std::wstring& name, uint32_t NumElements, uint32_t ElementSize, const UploadBuffer& SrcData, uint32_t SrcOffset = 0);

	void Create(const std::wstring& name, uint32_t NumElements, uint32_t ElementSize, EsramAllocator& Allocator,
		const void* initialData = nullptr);

	void CreatePlaced(const std::wstring& name, ID3D12Heap* pBackingHeap, uint32_t HeapOffset, uint32_t NumElements,
		uint32_t ElementSize, const void* initialData = nullptr);

	const D3D12_CPU_DESCRIPTOR_HANDLE& GetUAV(void) const { return Y_UAV; }
	const D3D12_CPU_DESCRIPTOR_HANDLE& GetSRV(void) const { return Y_SRV; }

	D3D12_GPU_VIRTUAL_ADDRESS RootConstantBufferView() const { return Y_GpuVirtualAddress; }

	D3D12_CPU_DESCRIPTOR_HANDLE CreateConstantBufferView(uint32_t Offset, uint32_t Size) const;

	D3D12_VERTEX_BUFFER_VIEW VertexBufferView(size_t Offset, uint32_t Size, uint32_t Stride) const;
	D3D12_VERTEX_BUFFER_VIEW VertexBufferView(size_t BaseVertexIndex = 0) const
	{
		uint32_t Offset = BaseVertexIndex * Y_ElementSize;
		return VertexBufferView(Offset, (uint32_t)(Y_BufferSize - Offset), Y_ElementSize);
	}

	D3D12_INDEX_BUFFER_VIEW IndexBufferView(size_t Offset, uint32_t Size, bool b32Bit = false) const;
	D3D12_INDEX_BUFFER_VIEW IndexBufferView(size_t StartIndex = 0) const
	{
		size_t Offset = StartIndex * Y_ElementSize;
		return IndexBufferView(Offset, (uint32_t)(Y_BufferSize - Offset), Y_ElementSize ==4);
	}

	size_t GetBufferSize() { return Y_BufferSize; }
	uint32_t GetElementCount() { return Y_ElementCount; }
	uint32_t GetElementSize() { return Y_ElementSize; }

protected:

	GpuBuffer(void) :
		Y_BufferSize(0),
		Y_ElementCount(0),
		Y_ElementSize(0)
	{
		Y_ResourceFlags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		Y_UAV.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
		Y_SRV.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
	}

	D3D12_RESOURCE_DESC DescribleBuffer(void);
	virtual void CreateDerivedViews(void) = 0;

	D3D12_CPU_DESCRIPTOR_HANDLE Y_UAV;
	D3D12_CPU_DESCRIPTOR_HANDLE Y_SRV;

	size_t Y_BufferSize;
	uint32_t Y_ElementCount;
	uint32_t Y_ElementSize;

	D3D12_RESOURCE_FLAGS Y_ResourceFlags;

};

inline D3D12_VERTEX_BUFFER_VIEW GpuBuffer::VertexBufferView(size_t Offset, uint32_t Size, uint32_t Stride) const
{
	D3D12_VERTEX_BUFFER_VIEW VBufferView;
	VBufferView.BufferLocation = Y_GpuVirtualAddress + Offset;
	VBufferView.SizeInBytes = Size;
	VBufferView.StrideInBytes = Stride;

	return VBufferView;
}

inline D3D12_INDEX_BUFFER_VIEW GpuBuffer::IndexBufferView(size_t Offset, uint32_t Size, bool b32Bit /* = false */) const
{
	D3D12_INDEX_BUFFER_VIEW IBufferView;
	IBufferView.BufferLocation = Y_GpuVirtualAddress + Offset;
	IBufferView.Format = b32Bit ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT;
	IBufferView.SizeInBytes = Size;

	return IBufferView;
}

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
		Y_CounterBuffer.Destroy();
		GpuBuffer::Destroy();
	}

	virtual void CreateDerivedViews(void)override;

	ByteAddressBuffer& GetCounterBuffer(void) { return Y_CounterBuffer; }

	const D3D12_CPU_DESCRIPTOR_HANDLE& GetCounterUAV(CommandContext& Context);
	const D3D12_CPU_DESCRIPTOR_HANDLE& GetCounterSRV(CommandContext& Context);

private:
	ByteAddressBuffer Y_CounterBuffer;
};

class TypedBuffer : public GpuBuffer
{
public:
	TypedBuffer(DXGI_FORMAT Format) : Y_DataFormat(Format)
	{

	}


	virtual void CreateDerivedViews(void)override;

private:
	DXGI_FORMAT Y_DataFormat;
};

