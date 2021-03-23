#pragma once

#include <mutex>
#include <vector>

class FDescriptorAllocator
{
public:
	FDescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE Type) :
		Type(Type),
		CurrentHeap(nullptr),
		DescriptorSize(0)
	{
		Currenthandle.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
	}


	D3D12_CPU_DESCRIPTOR_HANDLE Allocate(uint32_t Count);

	static void DestroyAll(void);

protected:

	static const uint32_t NumDescriptorsPerHeap = 256;
	static std::mutex AllocationMutex;
	static std::vector<Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>> DescriptorHeapPool;
	static ID3D12DescriptorHeap* RequestNewHeap(D3D12_DESCRIPTOR_HEAP_TYPE Type);

	D3D12_DESCRIPTOR_HEAP_TYPE Type;
	ID3D12DescriptorHeap* CurrentHeap;
	D3D12_CPU_DESCRIPTOR_HANDLE Currenthandle;
	uint32_t DescriptorSize;
	uint32_t RemainingFreeHandles;

};

class FDescriptorHandle
{
public:
	FDescriptorHandle()
	{
		CpuHandle.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
		GpuHandle.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
	}

	FDescriptorHandle(D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle) :
		CpuHandle(cpuHandle),
		GpuHandle(gpuHandle)
	{

	}

	FDescriptorHandle operator+ (INT OffsetScaledByDescriptorSize)const
	{
		FDescriptorHandle Ret = *this;
		Ret += OffsetScaledByDescriptorSize;

		return Ret;
	}

	void operator += (UINT OffsetScaledByDescriptorSize)
	{
		if (CpuHandle.ptr != D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
		{
			CpuHandle.ptr += OffsetScaledByDescriptorSize;
		}

		if (GpuHandle.ptr != D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
		{
			GpuHandle.ptr += OffsetScaledByDescriptorSize;
		}
	}

	const D3D12_CPU_DESCRIPTOR_HANDLE* operator&() const { return &CpuHandle; }
	operator D3D12_CPU_DESCRIPTOR_HANDLE() const { return CpuHandle; }
	operator D3D12_GPU_DESCRIPTOR_HANDLE() const { return GpuHandle; }

	size_t GetCpuPtr() const { return CpuHandle.ptr; }
	size_t GetGpuPtr() const { return GpuHandle.ptr; }

	bool IsNull() const { return CpuHandle.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN; }
	bool IsShaderVisible() const { return GpuHandle.ptr != D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN; }

private:
	D3D12_CPU_DESCRIPTOR_HANDLE CpuHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE GpuHandle;

};

class FDescriptorHeap
{
public:
	FDescriptorHeap(){}
	~FDescriptorHeap()
	{
		Destroy();
	}

	void Create(const std::wstring& DebugHeapName, D3D12_DESCRIPTOR_HEAP_TYPE Type, uint32_t MaxCount);
	void Destroy() { Heap = nullptr; }

	bool HasAvailableSpace(uint32_t Count)const { return Count < NumFreeDescriptors; }
	FDescriptorHandle Allocate(uint32_t Count = 1);

	FDescriptorHandle operator[](uint32_t ArrayIndex) const { return FirstHandle + ArrayIndex * DescriptorSize; }

	uint32_t GetOffsetOfHandle(const FDescriptorHandle& Handle){
		return (uint32_t)(Handle.GetCpuPtr() - FirstHandle.GetCpuPtr()) / DescriptorSize;
	}

	bool ValidateHandle(const FDescriptorHandle& Handle)const;

	ID3D12DescriptorHeap* GetHeapPointer()const { return Heap.Get(); }
	uint32_t GetDescriptorSize(void)const { return DescriptorSize; }

private:

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> Heap;
	D3D12_DESCRIPTOR_HEAP_DESC HeapDesc;
	uint32_t DescriptorSize;
	uint32_t NumFreeDescriptors;

	FDescriptorHandle FirstHandle;
	FDescriptorHandle NextFreeHandle;

};