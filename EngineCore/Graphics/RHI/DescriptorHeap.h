#pragma once

#include <mutex>
#include <vector>

class DescriptorAllocator
{
public:
	DescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE Type) :
		m_Type(Type),
		m_CurrentHeap(nullptr),
		m_DescriptorSize(0)
	{
		m_CurrentHandle.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
	}


	D3D12_CPU_DESCRIPTOR_HANDLE Allocate(uint32_t Count);

	static void DestroyAll(void);

protected:

	static const uint32_t sm_NumDescriptorsPerHeap = 256;
	static std::mutex sm_AllocationMutex;
	static std::vector<Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>> sm_DescriptorHeapPool;
	static ID3D12DescriptorHeap* RequestNewHeap(D3D12_DESCRIPTOR_HEAP_TYPE Type);

	D3D12_DESCRIPTOR_HEAP_TYPE m_Type;
	ID3D12DescriptorHeap* m_CurrentHeap;
	D3D12_CPU_DESCRIPTOR_HANDLE m_Currenthandle;
	uint32_t m_DescriptorSize;
	uint32_t m_RemainingFreeHandles;

};

class DescriptorHandle
{
public:
	DescriptorHandle()
	{
		m_CpuHandle.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
		m_GpuHandle.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
	}

	DescriptorHandle(D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle) :
		m_CpuHandle(cpuHandle),
		m_GpuHandle(gpuHandle)
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
		if (m_CpuHandle.ptr != D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
		{
			m_CpuHandle.ptr += OffsetScaledByDescriptorSize;
		}

		if (m_GpuHandle.ptr != D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
		{
			m_GpuHandle.ptr += OffsetScaledByDescriptorSize;
		}
	}

	const D3D12_CPU_DESCRIPTOR_HANDLE* operator&() const { return &m_CpuHandle; }
	operator D3D12_CPU_DESCRIPTOR_HANDLE() const { return m_CpuHandle; }
	operator D3D12_GPU_DESCRIPTOR_HANDLE() const { return m_GpuHandle; }

	size_t GetCpuPtr() const { return m_CpuHandle.ptr; }
	size_t GetGpuPtr() const { return m_GpuHandle.ptr; }

	bool IsNull() const { return m_CpuHandle.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN; }
	bool IsShaderVisible() const { return m_GpuHandle.ptr != D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN; }

private:
	D3D12_CPU_DESCRIPTOR_HANDLE m_CpuHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE m_GpuHandle;

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
	void Destroy() { m_Heap = nullptr; }

	bool HasAvailableSpace(uint32_t Count)const { return Count < m_NumFreeDescriptors; }
	DescriptorHandle Allocate(uint32_t Count = 1);

	DescriptorHandle operator[](uint32_t ArrayIndex) const { return m_FirstHandle + ArrayIndex * m_DescriptorSize; }

	uint32_t GetOffsetOfHandle(const DescriptorHandle& Handle){
		return (uint32_t)(Handle.GetCpuPtr() - m_FirstHandle.GetCpuPtr()) / m_DescriptorSize;
	}

	bool ValidateHandle(const DescriptorHandle& Handle)const;

	ID3D12DescriptorHeap* GetHeapPointer()const { return m_Heap.Get(); }
	uint32_t GetDescriptorSize(void)const { return m_DescriptorSize; }

private:

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_Heap;
	D3D12_DESCRIPTOR_HEAP_DESC m_HeapDesc;
	uint32_t m_DescriptorSize;
	uint32_t m_NumFreeDescriptors;

	DescriptorHandle m_FirstHandle;
	DescriptorHandle m_NextFreeHandle;

};