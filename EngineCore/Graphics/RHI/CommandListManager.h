#pragma once
#include "../../pch.h"
#include "CommandAllocatorPool.h"

class FCommandQueue
{
	friend class FCommandContext;
	friend class FCommandListmanager;
public:
	FCommandQueue(D3D12_COMMAND_LIST_TYPE Type);
	~FCommandQueue();

	void Create(ID3D12Device* Device);
	void ShutDown();

	inline bool IsReady()
	{
		return CommandQueue != nullptr;
	}

	//for fence event
	uint64_t IncrementFence(void);
	bool IsFenceComplete(uint64_t FenceValue);
	void StallForFence(uint64_t FenceValue);
	void StallForProducer(FCommandQueue& Producer);
	void WaitForFence(uint64_t FenceValue);
	void WaitForIdle(void) { WaitForFence(IncrementFence()); }


	ID3D12CommandQueue* GetCommandQueue()
	{
		return CommandQueue;
	}

	uint64_t GetNextFenceValue() { return NexFenceValue; }

private:

	uint64_t ExcuteCommandList(ID3D12CommandList* List);
	ID3D12CommandAllocator* RequestAllocator(void);
	void DiscardAllocator(uint64_t FenceValueForReset, ID3D12CommandAllocator* Allocator);

	FCommandAllocatorPool AllocatorPool;
	std::mutex FenceMutex;
	std::mutex EventMutex;

	D3D12_COMMAND_LIST_TYPE CommandListType;
	ID3D12CommandQueue* CommandQueue;

	//life time of these objects is managed by the descriptor cache
	ID3D12Fence* Fence;
	uint64_t NexFenceValue;
	uint64_t LastCompletedFenceValue;
	HANDLE FenceEventHandle;
};


class FCommandListmanager
{
public:
	FCommandListmanager();
	~FCommandListmanager();

	void Create(ID3D12Device* InDevice);
	void ShutDown();

	FCommandQueue& GetGraphicsQueue(void) { return GraphicsQueue; }
	FCommandQueue& GetComputeQueue(void) { return ComputeQueue; }
	FCommandQueue& GetCopyQueue(void) { return CopyQueue; }

	FCommandQueue& GetQueue(D3D12_COMMAND_LIST_TYPE Type)
	{
		switch (Type)
		{
		case D3D12_COMMAND_LIST_TYPE_DIRECT:
			return GraphicsQueue;
		//case D3D12_COMMAND_LIST_TYPE_BUNDLE:
		//	break;
		case D3D12_COMMAND_LIST_TYPE_COMPUTE:
			return ComputeQueue;
		//case D3D12_COMMAND_LIST_TYPE_COPY:
		//	break;
		//case D3D12_COMMAND_LIST_TYPE_VIDEO_DECODE:
		//	break;
		//case D3D12_COMMAND_LIST_TYPE_VIDEO_PROCESS:
		//	break;
		//case D3D12_COMMAND_LIST_TYPE_VIDEO_ENCODE:
		//	break;
		default:
			return GraphicsQueue;
		}
	}

	ID3D12CommandQueue* GetCommandQueue()
	{
		return GraphicsQueue.GetCommandQueue();
	}


	void CreateNewCommandList(D3D12_COMMAND_LIST_TYPE Type, ID3D12GraphicsCommandList** List, ID3D12CommandAllocator** Allocator);

	bool IsFenceComplete(uint64_t FenceValue)
	{
		return GetQueue(D3D12_COMMAND_LIST_TYPE(FenceValue >> 56)).IsFenceComplete(FenceValue);
	}

	void WaitForFence(uint64_t FenceValue);

	void IdleGPU(void)
	{
		GraphicsQueue.WaitForIdle();
		ComputeQueue.WaitForIdle();
		CopyQueue.WaitForIdle();
	}

private:
	ID3D12Device* Device;

	FCommandQueue GraphicsQueue;
	FCommandQueue ComputeQueue;
	FCommandQueue CopyQueue;
};
