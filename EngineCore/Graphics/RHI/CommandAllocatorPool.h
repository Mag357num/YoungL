#pragma once

#include <vector>
#include <mutex>
#include <queue>

#include "../../pch.h"

class FCommandAllocatorPool
{
public:
	FCommandAllocatorPool(D3D12_COMMAND_LIST_TYPE Type);
	~FCommandAllocatorPool();

	void Create(ID3D12Device* pDevice);
	void ShutDown();

	ID3D12CommandAllocator* RequestAllocator(uint64_t CompletedFenceEvent);
	void DiscardAllocator(uint64_t FenceValue, ID3D12CommandAllocator* Allocator);

	inline size_t Size() { return AllocatorPool.size(); }

private:
	const D3D12_COMMAND_LIST_TYPE CommandListType;

	ID3D12Device* Device;
	std::vector<ID3D12CommandAllocator*> AllocatorPool;

	std::queue<std::pair<uint64_t, ID3D12CommandAllocator*>> ReadyAllocator;
	std::mutex AllocatorMutex;
};
