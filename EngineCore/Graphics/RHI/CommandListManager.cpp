#include "../../pch.h"
#include "CommandListManager.h"

CommandQueue::CommandQueue(D3D12_COMMAND_LIST_TYPE Type)
	:m_CommandListType(Type),
	m_CommandQueue(nullptr),
	m_pFence(nullptr),
	m_NexFenceValue((uint64_t)Type << 56 | 1),
	m_LastCompletedFenceValue((uint64_t)Type << 56),
	m_AllocatorPool(Type)
{

}

CommandQueue::~CommandQueue()
{
	ShutDown();
}

void CommandQueue::ShutDown()
{
	if (m_CommandQueue == nullptr)
	{
		return;
	}

	m_AllocatorPool.ShutDown();
	CloseHandle(m_FenceEventHandle);

	m_pFence->Release();
	m_pFence = nullptr;

	m_CommandQueue->Release();
	m_CommandQueue = nullptr;
}

void CommandQueue::Create(ID3D12Device* Device)
{
	ASSERT(Device != nullptr);
	ASSERT(IsReady());
	ASSERT(m_AllocatorPool.Size() == 0);

	D3D12_COMMAND_QUEUE_DESC QueuDesc = {};
	QueuDesc.Type = m_CommandListType;
	QueuDesc.NodeMask = 1;
	Device->CreateCommandQueue(&QueuDesc, MY_IID_PPV_ARGS(&m_CommandQueue));
	m_CommandQueue->SetName(L"CommandListmanager::m_CommandQueue");

	ASSERT_SUCCEEDED(Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, MY_IID_PPV_ARGS(&m_pFence)));
	m_pFence->SetName(L"CommandListManager::m_pFence");
	m_pFence->Signal((uint64_t)m_CommandListType << 56);

	m_FenceEventHandle = CreateEvent(nullptr, false, false, nullptr);
	ASSERT(m_FenceEventHandle != NULL);

	m_AllocatorPool.Create(Device);

	ASSERT(IsReady());

}

void CommandQueue::IncrementFence()
{
	std::lock_guard<std::mutex> LockGuard(m_FenceMutex);
	m_CommandQueue->Signal(m_pFence, m_NexFenceValue);
	m_NexFenceValue++;
}

bool CommandQueue::IsFenceComplete(uint64_t FenceValue)
{
	//avoid querying the fence value by testing against the last one seen.
	if (FenceValue > m_LastCompletedFenceValue)
	{
		m_LastCompletedFenceValue = std::max(m_LastCompletedFenceValue, m_pFence->GetCompletedValue());
	}

	return FenceValue <= m_LastCompletedFenceValue;
}

uint64_t CommandQueue::ExcuteCommandList(ID3D12CommandList* List)
{

	std::lock_guard<std::mutex> LockGuard(m_FenceMutex);
	ASSERT_SUCCEEDED(((ID3D12GraphicsCommandList*)List).Close());

	m_CommandQueue->ExecuteCommandLists(1, &List);
	m_CommandQueue->Signal(m_pFence, m_NexFenceValue);

	return m_NexFenceValue++;
}


ID3D12CommandAllocator* CommandQueue::RequestAllocator()
{
	uint64_t CompletedFence = m_pFence->GetCompletedValue();
	return m_AllocatorPool.RequestAllocator(CompletedFence);

}

void CommandQueue::DiscardAllocator(uint64_t FenceValueForReset, ID3D12CommandAllocator* Allocator)
{
	m_AllocatorPool.DiscardAllocator(FenceValueForReset, Allocator);
}


namespace Graphics
{
	extern CommandListmanager g_CommandManager;
}

void CommandQueue::StallForFence(uint64_t FenceValue)
{
	CommandQueue& Producer = Graphics::g_CommandManager.GetQueue((D3D12_COMMAND_LIST_TYPE)(FenceValue >> 56));
	m_CommandQueue->Wait(Producer.m_pFence, FenceValue);
}

void CommandQueue::StallForProducer(CommandQueue& Producer)
{
	ASSERT(Producer.m_NexFenceValue > 0);
	m_CommandQueue->Wait(Producer.m_pFence, Producer.m_NexFenceValue - 1);
}

void CommandQueue::WaitForFence(uint64_t FenceValue)
{
	if (IsFenceComplete(FenceValue))
	{
		return;
	}

	//
	{
		std::lock_guard<std::mutex> LockGuard(m_EventMutex);
		m_pFence->SetEventOnCompletion(FenceValue, m_FenceEventHandle);
		WaitForSingleObject(m_FenceEventHandle, INFINITE);
		m_LastCompletedFenceValue = FenceValue;
	}
}


CommandListmanager::CommandListmanager()
	:m_Device(nullptr),
	m_GraphicsQueue(D3D12_COMMAND_LIST_TYPE_DIRECT),
	m_ComputeQueue(D3D12_COMMAND_LIST_TYPE_COMPUTE),
	m_CopyQueue(D3D12_COMMAND_LIST_TYPE_COPY)
{

}

CommandListmanager::~CommandListmanager()
{
	ShutDown();
}


void CommandListmanager::Create(ID3D12Device* Device)
{
	ASSERT(Device != nullptr);

	m_Device = Device;

	m_GraphicsQueue.Create(Device);
	m_ComputeQueue.Create(Device);
	m_CopyQueue.Create(Device);
}

void CommandListmanager::CreateNewCommandList(D3D12_COMMAND_LIST_TYPE Type, ID3D12GraphicsCommandList** List, ID3D12CommandAllocator** Allocator)
{
	ASSERT(Type != D3D12_COMMAND_LIST_TYPE_BUNDLE, "Boundles are not yet supported!");
	switch (Type)
	{
	case D3D12_COMMAND_LIST_TYPE_DIRECT:
		*Allocator = m_GraphicsQueue.RequestAllocator();
		break;
	//case D3D12_COMMAND_LIST_TYPE_BUNDLE:
	//	break;
	case D3D12_COMMAND_LIST_TYPE_COMPUTE:
		*Allocator = m_ComputeQueue.RequestAllocator();
		break;
	case D3D12_COMMAND_LIST_TYPE_COPY:
		*Allocator = m_CopyQueue.RequestAllocator();
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

	m_Device->CreateCommandList(1, Type, *Allocator, NULL, MY_IID_PPV_ARGS(List));
}

void CommandListmanager::ShutDown()
{
	m_ComputeQueue.ShutDown();
	m_GraphicsQueue.ShutDown();
	m_CopyQueue.ShutDown();
}

void CommandListmanager::WaitForFence(uint64_t FenceValue)
{
	CommandQueue& Producer = Graphics::g_CommandManager.GetQueue((D3D12_COMMAND_LIST_TYPE)(FenceValue >> 56));
	Producer.WaitForFence(FenceValue);
}
