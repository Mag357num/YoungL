#include "../../pch.h"
#include "DescriptorHeap.h"

using namespace Graphics;

std::mutex DescriptorAllocator::sm_AllocationMutex;
std::vector<Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>> DescriptorAllocator::sm_DescriptorHeapPool;

void DescriptorAllocator::DestroyAll()
{
	sm_DescriptorHeapPool.clear();
}

ID3D12DescriptorHeap* DescriptorAllocator::RequestNewHeap(D3D12_DESCRIPTOR_HEAP_TYPE Type)
{
	std::lock_guard<std::mutex> LockGuard(sm_AllocationMutex);

	D3D12_DESCRIPTOR_HEAP_DESC Desc;
	Desc.Type = Type;
	Desc.NumDescriptors = sm_NumDescriptorsPerHeap;
	Desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	Desc.NodeMask = 1;

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> pHeap;
	ASSERT_SUCCEEDED(Graphics::g_Device->CreateDescriptorHeap(&Desc, MY_IID_PPV_ARGS(&pHeap)));

	sm_DescriptorHeapPool.emplace_back(pHeap);
	return pHeap.Get();
}

void DescriptorAllocator::Allocate(uint32_t Count)
{
	if (m_CurrentHeap == nullptr| m_RemainingFreeHandles < Count)
	{
		m_CurrentHeap = RequestNewHeap(m_Type);
		m_CurrentHandle = m_CurrentHeap->GetCPUDescriptorHandleForHeapStart();
		m_RemainingFreeHandles = sm_NumDescriptorsPerHeap;

		if (m_DescriptorSize ==0)
		{
			m_DescriptorSize = Graphics::g_Device->GetDescriptorHandleIncrementSize(m_Type);
		}
	}

	D3D12_CPU_DESCRIPTOR_HANDLE Ret = m_CurrentHandle;
	m_CurrentHandle.ptr += Count * m_DescriptorSize;
	m_RemainingFreeHandles -= Count;

	return Ret;

}

void DescriptorHeap::Create(const std::wstring& DebugHeapName, D3D12_DESCRIPTOR_HEAP_TYPE Type, uint32_t MaxCount)
{
	m_HeapDesc.Type = Type;
	m_HeapDesc.NumDescriptors = MaxCount;
	m_HeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	m_HeapDesc.NodeMask = 1;

	ASSERT_SUCCEEDED(Graphics::g_Device->CreateDescriptorHeap(&m_HeapDesc, MY_IID_PPV_ARGS(&m_Heap));

#ifdef RELEASE
	(void)Name;

#else
	m_Heap->SetName(DebugHeapName);
#endif

	m_DescriptorSize = Graphics::g_Device->GetDescriptorHandleIncrementSize(Type);
	m_NumFreeDescriptors = m_HeapDesc.NumDescriptors;
	m_FirstHandle = DescriptorHandle(
	m_Heap->GetCPUDescriptorHandleForHeapStart(),
		m_Heap->GetGPUDescriptorHandleForHeapStart()
	);

	m_NextFreeHandle = m_FirstHandle;
}


DescriptorHandle DescriptorHeap::Allocate(uint32_t Count /* = 1 */)
{
	ASSERT(HasAvailableSpace(Count), "Descriptor heap out of space , increse heap size");
	DescriptorHandle Ret = m_NextFreeHandle;
	m_NextFreeHandle += Count * m_DescriptorSize;
	m_NumFreeDescriptors -= Count;

	return Ret;
}

bool DescriptorHeap::ValidateHandle(const DescriptorHandle& Handle)const
{
	if (Handle.GetCpuPtr() < m_FirstHandle.GetCpuPtr() ||
		Handle.GetCpuPtr() >= m_FirstHandle.GetCpuPtr() + m_HeapDesc.NumDescriptors * m_DescriptorSize)
	{
		return false;
	}

	if (Handle.GetGpuPtr() - m_FirstHandle.GetGpuPtr() != 
		Handle.GetCpuPtr() - m_FirstHandle.GetCpuPtr())
	{
		return false;
	}

	return true;
}
