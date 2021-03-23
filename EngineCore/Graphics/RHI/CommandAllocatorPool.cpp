#include "../../pch.h"
#include "CommandAllocatorPool.h"

FCommandAllocatorPool::FCommandAllocatorPool(D3D12_COMMAND_LIST_TYPE Type)
	:CommandListType(Type),
	Device(nullptr)
{

}

FCommandAllocatorPool::~FCommandAllocatorPool()
{
	ShutDown();
}

void FCommandAllocatorPool::Create(ID3D12Device* pDevice)
{
	Device = pDevice;
}

void FCommandAllocatorPool::ShutDown()
{
	for (size_t i = 0; i < AllocatorPool.size(); ++i)
	{
		AllocatorPool[i]->Release();
	}

	AllocatorPool.clear();
}

ID3D12CommandAllocator* FCommandAllocatorPool::RequestAllocator(uint64_t CompletedFenceEvent)
{
	std::lock_guard<std::mutex> LockGuard(AllocatorMutex);
	ID3D12CommandAllocator* pAllocator = nullptr;

	if (!ReadyAllocator.empty())
	{
		std::pair<uint64_t, ID3D12CommandAllocator*>& AllocatorPair = ReadyAllocator.front();
		if (AllocatorPair.first <= CompletedFenceEvent)
		{
			pAllocator = AllocatorPair.second;
			ASSERT_SUCCEEDED(pAllocator->Reset());
			ReadyAllocator.pop();
		}
	}

	if (pAllocator == nullptr)
	{
		ASSERT_SUCCEEDED(Device->CreateCommandAllocator(CommandListType, MY_IID_PPV_ARGS(&pAllocator)));

		wchar_t AllocatorName[32];
		swprintf(AllocatorName, 32, L"CommandAllocator %zu", AllocatorPool.size());
		pAllocator->SetName(AllocatorName);
		AllocatorPool.push_back(pAllocator);
	}

	return pAllocator;
}

void FCommandAllocatorPool::DiscardAllocator(uint64_t FenceValue, ID3D12CommandAllocator* Allocator)
{
	std::lock_guard<std::mutex> LockGuard(AllocatorMutex);
	ReadyAllocator.push(std::make_pair(FenceValue, Allocator));
}