#include "../../pch.h"
#include "CommandListManager.h"

FCommandQueue::FCommandQueue(D3D12_COMMAND_LIST_TYPE Type)
	:CommandListType(Type),
	CommandQueue(nullptr),
	Fence(nullptr),
	NexFenceValue((uint64_t)Type << 56 | 1),
	LastCompletedFenceValue((uint64_t)Type << 56),
	AllocatorPool(Type)
{

}

FCommandQueue::~FCommandQueue()
{
	ShutDown();
}

void FCommandQueue::ShutDown()
{
	if (CommandQueue == nullptr)
	{
		return;
	}

	AllocatorPool.ShutDown();
	CloseHandle(FenceEventHandle);

	Fence->Release();
	Fence = nullptr;

	CommandQueue->Release();
	CommandQueue = nullptr;
}

void FCommandQueue::Create(ID3D12Device* Device)
{
	ASSERT(Device != nullptr);
	ASSERT(IsReady());
	ASSERT(AllocatorPool.Size() == 0);

	D3D12_COMMAND_QUEUE_DESC QueuDesc = {};
	QueuDesc.Type = CommandListType;
	QueuDesc.NodeMask = 1;
	Device->CreateCommandQueue(&QueuDesc, MY_IID_PPV_ARGS(&CommandQueue));
	CommandQueue->SetName(L"CommandListmanager::m_FCommandQueue");

	ASSERT_SUCCEEDED(Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, MY_IID_PPV_ARGS(&Fence)));
	Fence->SetName(L"CommandListManager::m_pFence");
	Fence->Signal((uint64_t)CommandListType << 56);

	FenceEventHandle = CreateEvent(nullptr, false, false, nullptr);
	ASSERT(FenceEventHandle != NULL);

	AllocatorPool.Create(Device);

	ASSERT(IsReady());

}

uint64_t FCommandQueue::IncrementFence()
{
	std::lock_guard<std::mutex> LockGuard(FenceMutex);
	CommandQueue->Signal(Fence, NexFenceValue);
	return NexFenceValue++;
}

bool FCommandQueue::IsFenceComplete(uint64_t FenceValue)
{
	//avoid querying the fence value by testing against the last one seen.
	if (FenceValue > LastCompletedFenceValue)
	{
		LastCompletedFenceValue = std::max(LastCompletedFenceValue, Fence->GetCompletedValue());
	}

	return FenceValue <= LastCompletedFenceValue;
}

uint64_t FCommandQueue::ExcuteCommandList(ID3D12CommandList* List)
{
	std::lock_guard<std::mutex> LockGuard(FenceMutex);
	ASSERT_SUCCEEDED(((ID3D12GraphicsCommandList*)List)->Close());

	CommandQueue->ExecuteCommandLists(1, &List);
	CommandQueue->Signal(Fence, NexFenceValue);

	return NexFenceValue++;
}


ID3D12CommandAllocator* FCommandQueue::RequestAllocator()
{
	uint64_t CompletedFence = Fence->GetCompletedValue();
	return AllocatorPool.RequestAllocator(CompletedFence);

}

void FCommandQueue::DiscardAllocator(uint64_t FenceValueForReset, ID3D12CommandAllocator* Allocator)
{
	AllocatorPool.DiscardAllocator(FenceValueForReset, Allocator);
}


namespace Graphics
{
	extern FCommandListmanager g_CommandManager;
}

void FCommandQueue::StallForFence(uint64_t FenceValue)
{
	FCommandQueue& Producer = Graphics::g_CommandManager.GetQueue((D3D12_COMMAND_LIST_TYPE)(FenceValue >> 56));
	CommandQueue->Wait(Producer.Fence, FenceValue);
}

void FCommandQueue::StallForProducer(FCommandQueue& Producer)
{
	ASSERT(Producer.NexFenceValue > 0);
	CommandQueue->Wait(Producer.Fence, Producer.NexFenceValue - 1);
}

void FCommandQueue::WaitForFence(uint64_t FenceValue)
{
	if (IsFenceComplete(FenceValue))
	{
		return;
	}

	//
	{
		std::lock_guard<std::mutex> LockGuard(EventMutex);
		Fence->SetEventOnCompletion(FenceValue, FenceEventHandle);
		WaitForSingleObject(FenceEventHandle, INFINITE);
		LastCompletedFenceValue = FenceValue;
	}
}


FCommandListmanager::FCommandListmanager()
	:Device(nullptr),
	GraphicsQueue(D3D12_COMMAND_LIST_TYPE_DIRECT),
	ComputeQueue(D3D12_COMMAND_LIST_TYPE_COMPUTE),
	CopyQueue(D3D12_COMMAND_LIST_TYPE_COPY)
{

}

FCommandListmanager::~FCommandListmanager()
{
	ShutDown();
}


void FCommandListmanager::Create(ID3D12Device* Device)
{
	ASSERT(Device != nullptr);

	Device = Device;

	GraphicsQueue.Create(Device);
	ComputeQueue.Create(Device);
	CopyQueue.Create(Device);
}

void FCommandListmanager::CreateNewCommandList(D3D12_COMMAND_LIST_TYPE Type, ID3D12GraphicsCommandList** List, ID3D12CommandAllocator** Allocator)
{
	ASSERT(Type != D3D12_COMMAND_LIST_TYPE_BUNDLE, "Boundles are not yet supported!");
	switch (Type)
	{
	case D3D12_COMMAND_LIST_TYPE_DIRECT:
		*Allocator = GraphicsQueue.RequestAllocator();
		break;
	//case D3D12_COMMAND_LIST_TYPE_BUNDLE:
	//	break;
	case D3D12_COMMAND_LIST_TYPE_COMPUTE:
		*Allocator = ComputeQueue.RequestAllocator();
		break;
	case D3D12_COMMAND_LIST_TYPE_COPY:
		*Allocator = CopyQueue.RequestAllocator();
		break;
	//case D3D12_COMMAND_LIST_TYPE_VIDEO_DECODE:
	//	break;
	//case D3D12_COMMAND_LIST_TYPE_VIDEO_PROCESS:
	//	break;
	//case D3D12_COMMAND_LIST_TYPE_VIDEO_ENCODE:
	//	break;
	default:
		break;
	}

	Device->CreateCommandList(1, Type, *Allocator, NULL, MY_IID_PPV_ARGS(List));
}

void FCommandListmanager::ShutDown()
{
	ComputeQueue.ShutDown();
	GraphicsQueue.ShutDown();
	CopyQueue.ShutDown();
}

void FCommandListmanager::WaitForFence(uint64_t FenceValue)
{
	FCommandQueue& Producer = Graphics::g_CommandManager.GetQueue((D3D12_COMMAND_LIST_TYPE)(FenceValue >> 56));
	Producer.WaitForFence(FenceValue);
}
