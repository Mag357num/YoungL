#include "../../pch.h"
#include "CommandAllocatorPool.h"

CommandAllocatorPool::CommandAllocatorPool(D3D12_COMMAND_LIST_TYPE Type)
	:m_CommandListType(Type),
	m_Device(nullptr)
{

}

CommandAllocatorPool::~CommandAllocatorPool()
{
	ShutDown();
}

void CommandAllocatorPool::Create(ID3D12Device* pDevice)
{
	m_Device = pDevice;
}

void CommandAllocatorPool::ShutDown()
{
	for (size_t i = 0; i < m_AllocatorPool.size(); ++i)
	{
		m_AllocatorPool[i]->Release();
	}

	m_AllocatorPool.clear();
}

ID3D12CommandAllocator* CommandAllocatorPool::RequestAllocator(uint64_t CompletedFenceEvent)
{
	std::lock_guard<std::mutex> LockGuard(m_AllocatorMutex);
	ID3D12CommandAllocator* pAllocator = nullptr;

	if (!m_ReadyAllocator.empty())
	{
		std::pair<uint64_t, ID3D12CommandAllocator*>& AllocatorPair = m_ReadyAllocator.front();
		if (AllocatorPair.first <= CompletedFenceEvent)
		{
			pAllocator = AllocatorPair.second;
			ASSERT_SUCCEEDED(pAllocator->Reset());
			m_ReadyAllocator.pop();
		}
	}

	if (pAllocator == nullptr)
	{
		ASSERT_SUCCEEDED(m_Device->CreateCommandAllocator(m_CommandListType, MY_IID_PPV_ARGS(&pAllocator)));

		wchar_t AllocatorName[32];
		swprintf(AllocatorName, 32, L"CommandAllocator %zu", m_AllocatorPool.size());
		pAllocator->SetName(AllocatorName);
		m_AllocatorPool.push_back(pAllocator);
	}

	return pAllocator;
}

void CommandAllocatorPool::DiscardAllocator(uint64_t FenceValue, ID3D12CommandAllocator* Allocator)
{
	std::lock_guard<std::mutex> LockGuard(m_AllocatorMutex);
	m_ReadyAllocator.push(std::make_pair(FenceValue, Allocator));
}