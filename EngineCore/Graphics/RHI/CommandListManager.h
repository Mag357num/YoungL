#pragma once

#include "CommandAllocatorPool.h"

class CommandQueue
{

	friend class CommandListmanager;
public:
	CommandQueue(D3D12_COMMAND_LIST_TYPE Type);
	~CommandQueue();

	void Create(ID3D12Device* Device);
	void ShutDown();

	inline bool IsReady()
	{
		return m_CommandQueue != nullptr;
	}

	//for fence event
	void IncrementFence(void);
	bool IsFenceComplete(uint64_t FenceValue);
	void StallForFence(uint64_t FenceValue);
	void StallForProducer(CommandQueue& Producer);
	void WaitForFence(uint64_t FenceValue);
	void WaitForIdle(void) { WaitForFence(IncrementFence()); }


	ID3D12CommandQueue* GetCommandQueue()
	{
		return m_CommandQueue;
	}

	uint64_t GetNextFenceValue() { return m_NexFenceValue; }

private:

	uint64_t ExcuteCommandList(ID3D12CommandList* List);
	ID3D12CommandAllocator* RequestAllocator(void);
	void DiscardAllocator(uint64_t FenceValueForReset, ID3D12CommandAllocator* Allocator);

	CommandAllocatorPool m_AllocatorPool;
	std::mutex m_FenceMutex;
	std::mutex m_EventMutex;

	D3D12_COMMAND_LIST_TYPE m_CommandListType;
	ID3D12CommandQueue* m_CommandQueue;

	//life time of these objects is managed by the descriptor cache
	ID3D12Fence* m_pFence;
	uint64_t m_NexFenceValue;
	uint64_t m_LastCompletedFenceValue;
	HANDLE m_FenceEventHandle;
};


class CommandListmanager
{
public:
	CommandListmanager();
	~CommandListmanager();

	void Create(ID3D12Device* Device);
	void ShutDown();

	CommandQueue& GetGraphicsQueue(void) { return m_GraphicsQueue; }
	CommandQueue& GetComputeQueue(void) { return m_ComputeQueue; }
	CommandQueue& GetCopyQueue(void) { return m_CopyQueue; }

	CommandQueue& GetQueue(D3D12_COMMAND_LIST_TYPE Type)
	{
		switch (Type)
		{
		case D3D12_COMMAND_LIST_TYPE_DIRECT:
			return m_GraphicsQueue;
		//case D3D12_COMMAND_LIST_TYPE_BUNDLE:
		//	break;
		case D3D12_COMMAND_LIST_TYPE_COMPUTE:
			return m_ComputeQueue;
		//case D3D12_COMMAND_LIST_TYPE_COPY:
		//	break;
		//case D3D12_COMMAND_LIST_TYPE_VIDEO_DECODE:
		//	break;
		//case D3D12_COMMAND_LIST_TYPE_VIDEO_PROCESS:
		//	break;
		//case D3D12_COMMAND_LIST_TYPE_VIDEO_ENCODE:
		//	break;
		default:
			return m_GraphicsQueue;
		}
	}

	ID3D12CommandQueue* GetCommandQueue()
	{
		return m_GraphicsQueue.GetCommandQueue();
	}


	void CreateNewCommandList(D3D12_COMMAND_LIST_TYPE Type, ID3D12GraphicsCommandList** List, ID3D12CommandAllocator** Allocator);

	bool IsFenceComplete(uint64_t FenceValue)
	{
		return GetQueue(D3D12_COMMAND_LIST_TYPE(FenceValue >> 56)).IsFenceComplete(FenceValue);
	}

	void WaitForFence(uint64_t FenceValue);

	void IdleGPU(void)
	{
		m_GraphicsQueue.WaitForIdle();
		m_ComputeQueue.WaitForIdle();
		m_CopyQueue.WaitForIdle();
	}

private:
	ID3D12Device* m_Device;

	CommandQueue m_GraphicsQueue;
	CommandQueue m_ComputeQueue;
	CommandQueue m_CopyQueue;
};
