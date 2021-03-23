#include "../../pch.h"
#include "DescriptorHeap.h"
#include "../GraphicsCore.h"

using namespace Graphics;

std::mutex FDescriptorAllocator::AllocationMutex;
std::vector<Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>> FDescriptorAllocator::DescriptorHeapPool;

void FDescriptorAllocator::DestroyAll()
{
	DescriptorHeapPool.clear();
}

ID3D12DescriptorHeap* FDescriptorAllocator::RequestNewHeap(D3D12_DESCRIPTOR_HEAP_TYPE Type)
{
	std::lock_guard<std::mutex> LockGuard(AllocationMutex);

	D3D12_DESCRIPTOR_HEAP_DESC Desc;
	Desc.Type = Type;
	Desc.NumDescriptors = NumDescriptorsPerHeap;
	Desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	Desc.NodeMask = 1;

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> pHeap;
	ASSERT_SUCCEEDED(Graphics::g_Device->CreateDescriptorHeap(&Desc, MY_IID_PPV_ARGS(&pHeap)));

	DescriptorHeapPool.emplace_back(pHeap);
	return pHeap.Get();
}

D3D12_CPU_DESCRIPTOR_HANDLE FDescriptorAllocator::Allocate(uint32_t Count)
{
	if (CurrentHeap == nullptr| RemainingFreeHandles < Count)
	{
		CurrentHeap = RequestNewHeap(Type);
		Currenthandle = CurrentHeap->GetCPUDescriptorHandleForHeapStart();
		RemainingFreeHandles = NumDescriptorsPerHeap;

		if (DescriptorSize ==0)
		{
			DescriptorSize = Graphics::g_Device->GetDescriptorHandleIncrementSize(Type);
		}
	}

	D3D12_CPU_DESCRIPTOR_HANDLE Ret = Currenthandle;
	Currenthandle.ptr += Count * DescriptorSize;
	RemainingFreeHandles -= Count;

	return Ret;

}

void FDescriptorHeap::Create(const std::wstring& DebugHeapName, D3D12_DESCRIPTOR_HEAP_TYPE Type, uint32_t MaxCount)
{
	HeapDesc.Type = Type;
	HeapDesc.NumDescriptors = MaxCount;
	HeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	HeapDesc.NodeMask = 1;

	ASSERT_SUCCEEDED(Graphics::g_Device->CreateDescriptorHeap(&HeapDesc, MY_IID_PPV_ARGS(&Heap)));

#ifdef RELEASE
	(void)Name;

#else
	Heap->SetName(DebugHeapName.c_str());
#endif

	DescriptorSize = Graphics::g_Device->GetDescriptorHandleIncrementSize(Type);
	NumFreeDescriptors = HeapDesc.NumDescriptors;
	FirstHandle = FDescriptorHandle(
		Heap->GetCPUDescriptorHandleForHeapStart(),
		Heap->GetGPUDescriptorHandleForHeapStart()
	);

	NextFreeHandle = FirstHandle;
}


FDescriptorHandle FDescriptorHeap::Allocate(uint32_t Count /* = 1 */)
{
	ASSERT(HasAvailableSpace(Count), "Descriptor heap out of space , increse heap size");
	FDescriptorHandle Ret = NextFreeHandle;
	NextFreeHandle += Count * DescriptorSize;
	NumFreeDescriptors -= Count;

	return Ret;
}

bool FDescriptorHeap::ValidateHandle(const FDescriptorHandle& Handle)const
{
	if (Handle.GetCpuPtr() < FirstHandle.GetCpuPtr() ||
		Handle.GetCpuPtr() >= FirstHandle.GetCpuPtr() + HeapDesc.NumDescriptors * DescriptorSize)
	{
		return false;
	}

	if (Handle.GetGpuPtr() - FirstHandle.GetGpuPtr() !=
		Handle.GetCpuPtr() - FirstHandle.GetCpuPtr())
	{
		return false;
	}

	return true;
}
