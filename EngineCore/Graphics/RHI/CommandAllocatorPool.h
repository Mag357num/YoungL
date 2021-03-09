#pragma once

#include <vector>
#include <mutex>
#include <queue>


class CommandAllocatorPool
{
public:
	CommandAllocatorPool(D3D12_COMMAND_LIST_TYPE Type);
	~CommandAllocatorPool();

	void Create(ID3D12Device* pDevice);
	void ShutDown();

	ID3D12CommandAllocator* RequestAllocator(uint64_t CompletedFenceEvent);
	void DiscardAllocator(uint64_t FenceValue, ID3D12CommandAllocator* Allocator);

	inline size_t Size() { return m_AllocatorPool.size(); }

private:
	const D3D12_COMMAND_LIST_TYPE m_CommandListType;

	ID3D12Device* m_Device;
	std::vector<ID3D12CommandAllocator*> m_AllocatorPool;

	std::queue<std::pair<uint64_t, ID3D12CommandAllocator*>> m_ReadyAllocator;
	std::mutex m_AllocatorMutex;
};
