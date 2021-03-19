#pragma once

#include <mutex>
#include <vector>

class DescriptorAllocator
{
public:
	DescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE Type) :
		Y_Type(Type),
		Y_CurrentHeap(nullptr),
		Y_DescriptorSize(0)
	{
		Y_Currenthandle.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
	}


	D3D12_CPU_DESCRIPTOR_HANDLE Allocate(uint32_t Count);

	static void DestroyAll(void);

protected:

	static const uint32_t Y_NumDescriptorsPerHeap = 256;
	static std::mutex Y_AllocationMutex;
	static std::vector<Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>> Y_DescriptorHeapPool;
	static ID3D12DescriptorHeap* RequestNewHeap(D3D12_DESCRIPTOR_HEAP_TYPE Type);

	D3D12_DESCRIPTOR_HEAP_TYPE Y_Type;
	ID3D12DescriptorHeap* Y_CurrentHeap;
	D3D12_CPU_DESCRIPTOR_HANDLE Y_Currenthandle;
	uint32_t Y_DescriptorSize;
	uint32_t Y_RemainingFreeHandles;

};

class DescriptorHandle
{
public:
	DescriptorHandle()
	{
		Y_CpuHandle.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
		Y_GpuHandle.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
	}

	DescriptorHandle(D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle) :
		Y_CpuHandle(cpuHandle),
		Y_GpuHandle(gpuHandle)
	{

	}

	DescriptorHandle operator+ (INT OffsetScaledByDescriptorSize)const
	{
		DescriptorHandle Ret = *this;
		Ret += OffsetScaledByDescriptorSize;

		return Ret;
	}

	void operator += (UINT OffsetScaledByDescriptorSize)
	{
		if (Y_CpuHandle.ptr != D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
		{
			Y_CpuHandle.ptr += OffsetScaledByDescriptorSize;
		}

		if (Y_GpuHandle.ptr != D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
		{
			Y_GpuHandle.ptr += OffsetScaledByDescriptorSize;
		}
	}

	const D3D12_CPU_DESCRIPTOR_HANDLE* operator&() const { return &Y_CpuHandle; }
	operator D3D12_CPU_DESCRIPTOR_HANDLE() const { return Y_CpuHandle; }
	operator D3D12_GPU_DESCRIPTOR_HANDLE() const { return Y_GpuHandle; }

	size_t GetCpuPtr() const { return Y_CpuHandle.ptr; }
	size_t GetGpuPtr() const { return Y_GpuHandle.ptr; }

	bool IsNull() const { return Y_CpuHandle.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN; }
	bool IsShaderVisible() const { return Y_GpuHandle.ptr != D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN; }

private:
	D3D12_CPU_DESCRIPTOR_HANDLE Y_CpuHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE Y_GpuHandle;

};

class DescriptorHeap
{
public:
	DescriptorHeap(){}
	~DescriptorHeap()
	{
		Destroy();
	}

	void Create(const std::wstring& DebugHeapName, D3D12_DESCRIPTOR_HEAP_TYPE Type, uint32_t MaxCount);
	void Destroy() { Y_Heap = nullptr; }

	bool HasAvailableSpace(uint32_t Count)const { return Count < Y_NumFreeDescriptors; }
	DescriptorHandle Allocate(uint32_t Count = 1);

	DescriptorHandle operator[](uint32_t ArrayIndex) const { return Y_FirstHandle + ArrayIndex * Y_DescriptorSize; }

	uint32_t GetOffsetOfHandle(const DescriptorHandle& Handle){
		return (uint32_t)(Handle.GetCpuPtr() - Y_FirstHandle.GetCpuPtr()) / Y_DescriptorSize;
	}

	bool ValidateHandle(const DescriptorHandle& Handle)const;

	ID3D12DescriptorHeap* GetHeapPointer()const { return Y_Heap.Get(); }
	uint32_t GetDescriptorSize(void)const { return Y_DescriptorSize; }

private:

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> Y_Heap;
	D3D12_DESCRIPTOR_HEAP_DESC Y_HeapDesc;
	uint32_t Y_DescriptorSize;
	uint32_t Y_NumFreeDescriptors;

	DescriptorHandle Y_FirstHandle;
	DescriptorHandle Y_NextFreeHandle;

};