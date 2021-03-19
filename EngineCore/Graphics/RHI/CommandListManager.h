#pragma once

#include "CommandAllocatorPool.h"

class CommandQueue
{
	friend class CommandContext;
	friend class CommandListmanager;
public:
	CommandQueue(D3D12_COMMAND_LIST_TYPE Type);
	~CommandQueue();

	void Create(ID3D12Device* Device);
	void ShutDown();

	inline bool IsReady()
	{
		return Y_CommandQueue != nullptr;
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
		return Y_CommandQueue;
	}

	uint64_t GetNextFenceValue() { return Y_NexFenceValue; }

private:

	uint64_t ExcuteCommandList(ID3D12CommandList* List);
	ID3D12CommandAllocator* RequestAllocator(void);
	void DiscardAllocator(uint64_t FenceValueForReset, ID3D12CommandAllocator* Allocator);

	CommandAllocatorPool Y_AllocatorPool;
	std::mutex Y_FenceMutex;
	std::mutex Y_EventMutex;

	D3D12_COMMAND_LIST_TYPE Y_CommandListType;
	ID3D12CommandQueue* Y_CommandQueue;

	//life time of these objects is managed by the descriptor cache
	ID3D12Fence* Y_pFence;
	uint64_t Y_NexFenceValue;
	uint64_t Y_LastCompletedFenceValue;
	HANDLE Y_FenceEventHandle;
};


class CommandListmanager
{
public:
	CommandListmanager();
	~CommandListmanager();

	void Create(ID3D12Device* Device);
	void ShutDown();

	CommandQueue& GetGraphicsQueue(void) { return Y_GraphicsQueue; }
	CommandQueue& GetComputeQueue(void) { return Y_ComputeQueue; }
	CommandQueue& GetCopyQueue(void) { return Y_CopyQueue; }

	CommandQueue& GetQueue(D3D12_COMMAND_LIST_TYPE Type)
	{
		switch (Type)
		{
		case D3D12_COMMAND_LIST_TYPE_DIRECT:
			return Y_GraphicsQueue;
		//case D3D12_COMMAND_LIST_TYPE_BUNDLE:
		//	break;
		case D3D12_COMMAND_LIST_TYPE_COMPUTE:
			return Y_ComputeQueue;
		//case D3D12_COMMAND_LIST_TYPE_COPY:
		//	break;
		//case D3D12_COMMAND_LIST_TYPE_VIDEO_DECODE:
		//	break;
		//case D3D12_COMMAND_LIST_TYPE_VIDEO_PROCESS:
		//	break;
		//case D3D12_COMMAND_LIST_TYPE_VIDEO_ENCODE:
		//	break;
		default:
			return Y_GraphicsQueue;
		}
	}

	ID3D12CommandQueue* GetCommandQueue()
	{
		return Y_GraphicsQueue.GetCommandQueue();
	}


	void CreateNewCommandList(D3D12_COMMAND_LIST_TYPE Type, ID3D12GraphicsCommandList** List, ID3D12CommandAllocator** Allocator);

	bool IsFenceComplete(uint64_t FenceValue)
	{
		return GetQueue(D3D12_COMMAND_LIST_TYPE(FenceValue >> 56)).IsFenceComplete(FenceValue);
	}

	void WaitForFence(uint64_t FenceValue);

	void IdleGPU(void)
	{
		Y_GraphicsQueue.WaitForIdle();
		Y_ComputeQueue.WaitForIdle();
		Y_CopyQueue.WaitForIdle();
	}

private:
	ID3D12Device* Y_Device;

	CommandQueue Y_GraphicsQueue;
	CommandQueue Y_ComputeQueue;
	CommandQueue Y_CopyQueue;
};
