#pragma once

#include "GPUResource.h"
#include "UploadBuffer.h"

class EsramAllocator;
class FCommandContext;

class FGpuBuffer : public FGPUResource
{
public:
	~FGpuBuffer() { Destroy(); }

	void Create(const std::wstring& Name, uint32_t NumElements, uint32_t ElementSize, const void* InitialData = nullptr);

	void Create(const std::wstring& Name, uint32_t NumElements, uint32_t ElementSize, const FUploadBuffer& SrcData, uint32_t SrcOffset = 0);

	void Create(const std::wstring& Name, uint32_t NumElements, uint32_t ElementSize, EsramAllocator& Allocator,
		const void* InitialData = nullptr);

	void CreatePlaced(const std::wstring& Name, ID3D12Heap* BackingHeap, uint32_t HeapOffset, uint32_t NumElements,
		uint32_t ElementSize, const void* InitialData = nullptr);

	const D3D12_CPU_DESCRIPTOR_HANDLE& GetUAV(void) const { return UAV; }
	const D3D12_CPU_DESCRIPTOR_HANDLE& GetSRV(void) const { return SRV; }

	D3D12_GPU_VIRTUAL_ADDRESS RootConstantBufferView() const { return GpuVirtualAddress; }

	D3D12_CPU_DESCRIPTOR_HANDLE CreateConstantBufferView(uint32_t Offset, uint32_t Size) const;

	D3D12_VERTEX_BUFFER_VIEW VertexBufferView(size_t Offset, uint32_t Size, uint32_t Stride) const;
	D3D12_VERTEX_BUFFER_VIEW VertexBufferView(uint32_t BaseVertexIndex = 0) const
	{
		uint32_t Offset = BaseVertexIndex * ElementSize;
		return VertexBufferView(Offset, (uint32_t)(BufferSize - Offset), ElementSize);
	}

	D3D12_INDEX_BUFFER_VIEW IndexBufferView(size_t Offset, uint32_t Size, bool b32Bit = false) const;
	D3D12_INDEX_BUFFER_VIEW IndexBufferView(size_t StartIndex = 0) const
	{
		size_t Offset = StartIndex * ElementSize;
		return IndexBufferView(Offset, (uint32_t)(BufferSize - Offset), ElementSize ==4);
	}

	size_t GetBufferSize() { return BufferSize; }
	uint32_t GetElementCount() { return ElementCount; }
	uint32_t GetElementSize() { return ElementSize; }

protected:

	FGpuBuffer(void) :
		BufferSize(0),
		ElementCount(0),
		ElementSize(0)
	{
		ResourceFlags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		UAV.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
		SRV.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
	}

	D3D12_RESOURCE_DESC DescribleBuffer(void);
	virtual void CreateDerivedViews(void) = 0;

	D3D12_CPU_DESCRIPTOR_HANDLE UAV;
	D3D12_CPU_DESCRIPTOR_HANDLE SRV;

	size_t BufferSize;
	uint32_t ElementCount;
	uint32_t ElementSize;

	D3D12_RESOURCE_FLAGS ResourceFlags;

};

inline D3D12_VERTEX_BUFFER_VIEW FGpuBuffer::VertexBufferView(size_t Offset, uint32_t Size, uint32_t Stride) const
{
	D3D12_VERTEX_BUFFER_VIEW VBufferView;
	VBufferView.BufferLocation = GpuVirtualAddress + Offset;
	VBufferView.SizeInBytes = Size;
	VBufferView.StrideInBytes = Stride;

	return VBufferView;
}

inline D3D12_INDEX_BUFFER_VIEW FGpuBuffer::IndexBufferView(size_t Offset, uint32_t Size, bool b32Bit /* = false */) const
{
	D3D12_INDEX_BUFFER_VIEW IBufferView;
	IBufferView.BufferLocation = GpuVirtualAddress + Offset;
	IBufferView.Format = b32Bit ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT;
	IBufferView.SizeInBytes = Size;

	return IBufferView;
}

class FByteAddressBuffer : public FGpuBuffer
{
public:
	virtual void CreateDerivedViews(void) override;
};

class FIndirectArgsBuffer : public FGpuBuffer
{
public:
	FIndirectArgsBuffer(void)
	{

	}
};

class FStructuredBuffer : public FGpuBuffer
{
public:
	virtual void Destroy()override
	{
		CounterBuffer.Destroy();
		FGpuBuffer::Destroy();
	}

	virtual void CreateDerivedViews(void)override;

	FByteAddressBuffer& GetCounterBuffer(void) { return CounterBuffer; }

	const D3D12_CPU_DESCRIPTOR_HANDLE& GetCounterUAV(FCommandContext& Context);
	const D3D12_CPU_DESCRIPTOR_HANDLE& GetCounterSRV(FCommandContext& Context);

private:
	FByteAddressBuffer CounterBuffer;
};

class FTypedBuffer : public FGpuBuffer
{
public:
	FTypedBuffer(DXGI_FORMAT Format) : DataFormat(Format)
	{

	}


	virtual void CreateDerivedViews(void)override;

private:
	DXGI_FORMAT DataFormat;
};

