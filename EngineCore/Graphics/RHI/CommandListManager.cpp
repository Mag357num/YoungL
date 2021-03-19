#include "../../pch.h"
#include "CommandListManager.h"

CommandQueue::CommandQueue(D3D12_COMMAND_LIST_TYPE Type)
	:Y_CommandListType(Type),
	Y_CommandQueue(nullptr),
	Y_pFence(nullptr),
	Y_NexFenceValue((uint64_t)Type << 56 | 1),
	Y_LastCompletedFenceValue((uint64_t)Type << 56),
	Y_AllocatorPool(Type)
{

}

CommandQueue::~CommandQueue()
{
	ShutDown();
}

void CommandQueue::ShutDown()
{
	if (Y_CommandQueue == nullptr)
	{
		return;
	}

	Y_AllocatorPool.ShutDown();
	CloseHandle(Y_FenceEventHandle);

	Y_pFence->Release();
	Y_pFence = nullptr;

	Y_CommandQueue->Release();
	Y_CommandQueue = nullptr;
}

void CommandQueue::Create(ID3D12Device* Device)
{
	ASSERT(Device != nullptr);
	ASSERT(IsReady());
	ASSERT(Y_AllocatorPool.Size() == 0);

	D3D12_COMMAND_QUEUE_DESC QueuDesc = {};
	QueuDesc.Type = Y_CommandListType;
	QueuDesc.NodeMask = 1;
	Device->CreateCommandQueue(&QueuDesc, MY_IID_PPV_ARGS(&Y_CommandQueue));
	Y_CommandQueue->SetName(L"CommandListmanager::m_CommandQueue");

	ASSERT_SUCCEEDED(Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, MY_IID_PPV_ARGS(&m_pFence)));
	Y_pFence->SetName(L"CommandListManager::m_pFence");
	Y_pFence->Signal((uint64_t)Y_CommandListType << 56);

	Y_FenceEventHandle = CreateEvent(nullptr, false, false, nullptr);
	ASSERT(Y_FenceEventHandle != NULL);

	Y_AllocatorPool.Create(Device);

	ASSERT(IsReady());

}

void CommandQueue::IncrementFence()
{
	std::lock_guard<std::mutex> LockGuard(Y_FenceMutex);
	Y_CommandQueue->Signal(Y_pFence, Y_NexFenceValue);
	Y_NexFenceValue++;
}

bool CommandQueue::IsFenceComplete(uint64_t FenceValue)
{
	//avoid querying the fence value by testing against the last one seen.
	if (FenceValue > Y_LastCompletedFenceValue)
	{
		Y_LastCompletedFenceValue = std::max(Y_LastCompletedFenceValue, Y_pFence->GetCompletedValue());
	}

	return FenceValue <= Y_LastCompletedFenceValue;
}

uint64_t CommandQueue::ExcuteCommandList(ID3D12CommandList* List)
{

	std::lock_guard<std::mutex> LockGuard(Y_FenceMutex);
	ASSERT_SUCCEEDED(((ID3D12GraphicsCommandList*)List).Close());

	Y_CommandQueue->ExecuteCommandLists(1, &List);
	Y_CommandQueue->Signal(Y_pFence, Y_NexFenceValue);

	return Y_NexFenceValue++;
}


ID3D12CommandAllocator* CommandQueue::RequestAllocator()
{
	uint64_t CompletedFence = Y_pFence->GetCompletedValue();
	return Y_AllocatorPool.RequestAllocator(CompletedFence);

}

void CommandQueue::DiscardAllocator(uint64_t FenceValueForReset, ID3D12CommandAllocator* Allocator)
{
	Y_AllocatorPool.DiscardAllocator(FenceValueForReset, Allocator);
}


namespace Graphics
{
	extern CommandListmanager g_CommandManager;
}

void CommandQueue::StallForFence(uint64_t FenceValue)
{
	CommandQueue& Producer = Graphics::g_CommandManager.GetQueue((D3D12_COMMAND_LIST_TYPE)(FenceValue >> 56));
	Y_CommandQueue->Wait(Producer.Y_pFence, FenceValue);
}

void CommandQueue::StallForProducer(CommandQueue& Producer)
{
	ASSERT(Producer.Y_NexFenceValue > 0);
	Y_CommandQueue->Wait(Producer.Y_pFence, Producer.Y_NexFenceValue - 1);
}

void CommandQueue::WaitForFence(uint64_t FenceValue)
{
	if (IsFenceComplete(FenceValue))
	{
		return;
	}

	//
	{
		std::lock_guard<std::mutex> LockGuard(Y_EventMutex);
		Y_pFence->SetEventOnCompletion(FenceValue, Y_FenceEventHandle);
		WaitForSingleObject(Y_FenceEventHandle, INFINITE);
		Y_LastCompletedFenceValue = FenceValue;
	}
}


CommandListmanager::CommandListmanager()
	:Y_Device(nullptr),
	Y_GraphicsQueue(D3D12_COMMAND_LIST_TYPE_DIRECT),
	Y_ComputeQueue(D3D12_COMMAND_LIST_TYPE_COMPUTE),
	Y_CopyQueue(D3D12_COMMAND_LIST_TYPE_COPY)
{

}

CommandListmanager::~CommandListmanager()
{
	ShutDown();
}


void CommandListmanager::Create(ID3D12Device* Device)
{
	ASSERT(Device != nullptr);

	Y_Device = Device;

	Y_GraphicsQueue.Create(Device);
	Y_ComputeQueue.Create(Device);
	Y_CopyQueue.Create(Device);
}

void CommandListmanager::CreateNewCommandList(D3D12_COMMAND_LIST_TYPE Type, ID3D12GraphicsCommandList** List, ID3D12CommandAllocator** Allocator)
{
	ASSERT(Type != D3D12_COMMAND_LIST_TYPE_BUNDLE, "Boundles are not yet supported!");
	switch (Type)
	{
	case D3D12_COMMAND_LIST_TYPE_DIRECT:
		*Allocator = Y_GraphicsQueue.RequestAllocator();
		break;
	//case D3D12_COMMAND_LIST_TYPE_BUNDLE:
	//	break;
	case D3D12_COMMAND_LIST_TYPE_COMPUTE:
		*Allocator = Y_ComputeQueue.RequestAllocator();
		break;
	case D3D12_COMMAND_LIST_TYPE_COPY:
		*Allocator = Y_CopyQueue.RequestAllocator();
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

	Y_Device->CreateCommandList(1, Type, *Allocator, NULL, MY_IID_PPV_ARGS(List));
}

void CommandListmanager::ShutDown()
{
	Y_ComputeQueue.ShutDown();
	Y_GraphicsQueue.ShutDown();
	Y_CopyQueue.ShutDown();
}

void CommandListmanager::WaitForFence(uint64_t FenceValue)
{
	CommandQueue& Producer = Graphics::g_CommandManager.GetQueue((D3D12_COMMAND_LIST_TYPE)(FenceValue >> 56));
	Producer.WaitForFence(FenceValue);
}
