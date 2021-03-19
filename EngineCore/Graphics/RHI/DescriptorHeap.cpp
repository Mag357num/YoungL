#include "../../pch.h"
#include "DescriptorHeap.h"

using namespace Graphics;

std::mutex DescriptorAllocator::Y_AllocationMutex;
std::vector<Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>> DescriptorAllocator::Y_DescriptorHeapPool;

void DescriptorAllocator::DestroyAll()
{
	Y_DescriptorHeapPool.clear();
}

ID3D12DescriptorHeap* DescriptorAllocator::RequestNewHeap(D3D12_DESCRIPTOR_HEAP_TYPE Type)
{
	std::lock_guard<std::mutex> LockGuard(Y_AllocationMutex);

	D3D12_DESCRIPTOR_HEAP_DESC Desc;
	Desc.Type = Type;
	Desc.NumDescriptors = Y_NumDescriptorsPerHeap;
	Desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	Desc.NodeMask = 1;

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> pHeap;
	ASSERT_SUCCEEDED(Graphics::g_Device->CreateDescriptorHeap(&Desc, MY_IID_PPV_ARGS(&pHeap)));

	Y_DescriptorHeapPool.emplace_back(pHeap);
	return pHeap.Get();
}

void DescriptorAllocator::Allocate(uint32_t Count)
{
	if (Y_CurrentHeap == nullptr| Y_RemainingFreeHandles < Count)
	{
		Y_CurrentHeap = RequestNewHeap(Y_Type);
		Y_Currenthandle = Y_CurrentHeap->GetCPUDescriptorHandleForHeapStart();
		Y_RemainingFreeHandles = Y_NumDescriptorsPerHeap;

		if (Y_DescriptorSize ==0)
		{
			Y_DescriptorSize = Graphics::g_Device->GetDescriptorHandleIncrementSize(Y_Type);
		}
	}

	D3D12_CPU_DESCRIPTOR_HANDLE Ret = Y_Currenthandle;
	Y_Currenthandle.ptr += Count * Y_DescriptorSize;
	Y_RemainingFreeHandles -= Count;

	return Ret;

}

void DescriptorHeap::Create(const std::wstring& DebugHeapName, D3D12_DESCRIPTOR_HEAP_TYPE Type, uint32_t MaxCount)
{
	Y_HeapDesc.Type = Type;
	Y_HeapDesc.NumDescriptors = MaxCount;
	Y_HeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	Y_HeapDesc.NodeMask = 1;

	ASSERT_SUCCEEDED(Graphics::g_Device->CreateDescriptorHeap(&Y_HeapDesc, MY_IID_PPV_ARGS(&Y_Heap));

#ifdef RELEASE
	(void)Name;

#else
	Y_Heap->SetName(DebugHeapName);
#endif

	Y_DescriptorSize = Graphics::g_Device->GetDescriptorHandleIncrementSize(Type);
	Y_NumFreeDescriptors = Y_HeapDesc.NumDescriptors;
	Y_FirstHandle = DescriptorHandle(
		Y_Heap->GetCPUDescriptorHandleForHeapStart(),
		Y_Heap->GetGPUDescriptorHandleForHeapStart()
	);

	Y_NextFreeHandle = Y_FirstHandle;
}


DescriptorHandle DescriptorHeap::Allocate(uint32_t Count /* = 1 */)
{
	ASSERT(HasAvailableSpace(Count), "Descriptor heap out of space , increse heap size");
	DescriptorHandle Ret = Y_NextFreeHandle;
	Y_NextFreeHandle += Count * Y_DescriptorSize;
	Y_NumFreeDescriptors -= Count;

	return Ret;
}

bool DescriptorHeap::ValidateHandle(const DescriptorHandle& Handle)const
{
	if (Handle.GetCpuPtr() < Y_FirstHandle.GetCpuPtr() ||
		Handle.GetCpuPtr() >= Y_FirstHandle.GetCpuPtr() + Y_HeapDesc.NumDescriptors * Y_DescriptorSize)
	{
		return false;
	}

	if (Handle.GetGpuPtr() - Y_FirstHandle.GetGpuPtr() !=
		Handle.GetCpuPtr() - Y_FirstHandle.GetCpuPtr())
	{
		return false;
	}

	return true;
}
