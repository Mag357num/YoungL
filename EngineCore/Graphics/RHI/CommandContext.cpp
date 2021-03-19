#include "../../pch.h"
#include "CommandContext.h"
#include "../GraphicsCore.h"
#include "../Buffer/GPUResource.h"

using namespace Graphics;

CommandContext* ContextManager::RequestContext(D3D12_COMMAND_LIST_TYPE Type)
{
	std::lock_guard<std::mutex> LockGuard(Y_ContextAvalibleMutex);
	CommandContext* Ret = nullptr;

	auto& AvailableComamndContext = Y_AvailableContextPool[Type];
	if (AvailableComamndContext.empty())
	{
		Ret = new CommandContext(Type);
		Y_ContextPool[Type].emplace_back(Ret);
		Ret->Initialize();
	}
	else
	{
		Ret = AvailableComamndContext.front();
		AvailableComamndContext.pop();
		Ret->Reset();
	}

	return Ret;
}

void ContextManager::ReleaseContext(CommandContext* UsedContext)
{
	ASSERT(UsedContext != nullptr);
	std::lock_guard<std::mutex> LockGuard(Y_ContextAvalibleMutex);
	Y_AvailableContextPool[UsedContext->Y_CommandType].push(UsedContext);
}

void ContextManager::DestroyAllContexts()
{
	for (uint32_t i=0; i< 4; i++)
	{
		Y_ContextPool[i].clear();
	}
}

CommandContext::CommandContext(D3D12_COMMAND_LIST_TYPE Type)
	:Y_CommandType(Type)
{
	Y_OwingCommandManager = nullptr;
	Y_CommandAllocator = nullptr;
	Y_CommandList = nullptr;
	ZeroMemory(Y_DescriptorHeaps, sizeof(Y_DescriptorHeaps));

	Y_GraphicsRootSignature = nullptr;
	Y_ComputeRootSignature = nullptr;
	Y_PipelineState = nullptr;
	Y_NumBarriesToFlush = 0;
}

CommandContext::~CommandContext(void)
{
	if (Y_CommandList != nullptr)
	{
		Y_CommandList->Release();
	}
}

void CommandContext::Initialize()
{
	g_CommandManager.CreateNewCommandList(Y_CommandType, &Y_CommandList, &Y_CommandAllocator);
}

void CommandContext::Reset()
{
	ASSERT(Y_CommandList != nullptr && Y_CommandAllocator == nullptr);
	Y_CommandAllocator = g_CommandManager.GetQueue(Y_CommandType).RequestAllocator();
	Y_CommandList->Reset(Y_CommandAllocator, nullptr);

	Y_GraphicsRootSignature = nullptr;
	Y_ComputeRootSignature = nullptr;
	Y_PipelineState = nullptr;
	Y_NumBarriesToFlush = 0;

	BuildDescriptorHeaps();
}


void CommandContext::BuildDescriptorHeaps()
{
	UINT NonHeaps = 0;
	ID3D12DescriptorHeap* HeapToBind[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
	for (UINT i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; i++)
	{
		ID3D12DescriptorHeap* Heap = Y_DescriptorHeaps[i];
		if (Heap != nullptr)
		{
			HeapToBind[NonHeaps++] = Heap;
		}
	}

	Y_CommandList->SetDescriptorHeaps(NonHeaps, HeapToBind);
}

uint64_t CommandContext::Flush(bool WaitForCompletion /* = false */)
{
	FlushResourceBarriers();
	ASSERT(Y_CommandAllocator != nullptr);

	uint64_t FenceValue = g_CommandManager.GetQueue(Y_CommandType).ExcuteCommandList(Y_CommandList);
	if (WaitForCompletion)
	{
		g_CommandManager.WaitForFence(FenceValue);
	}

	Y_CommandList->Reset(Y_CommandAllocator, nullptr);
	if (Y_GraphicsRootSignature)
	{
		Y_CommandList->SetGraphicsRootSignature(Y_GraphicsRootSignature);
	}

	if (Y_ComputeRootSignature)
	{
		Y_CommandList->SetComputeRootSignature(Y_ComputeRootSignature);
	}

	if (Y_PipelineState)
	{
		Y_CommandList->SetPipelineState(Y_PipelineState);
	}

	BuildDescriptorHeaps();
	return FenceValue;

}

uint64_t CommandContext::Finish(bool WaitForCompletion /* = false */)
{
	ASSERT(Y_CommandType == D3D12_COMMAND_LIST_TYPE_DIRECT | D3D12_COMMAND_LIST_TYPE_COMPUTE);
	FlushResourceBarriers();

	CommandQueue& CmdQueue = g_CommandManager.GetQueue(Y_CommandType);
	uint64_t FenceValue = CmdQueue.ExcuteCommandList(Y_CommandList);
	CmdQueue.DiscardAllocator(FenceValue, Y_CommandAllocator);
	Y_CommandAllocator = nullptr;

	if (WaitForCompletion)
	{
		g_CommandManager.WaitForFence(FenceValue);
	}
	g_ContextManager.ReleaseContext(this);

	return FenceValue;
}

void CommandContext::DestroyAllContexts()
{
	g_ContextManager.DestroyAllContexts();
}

void CommandContext::InitializeBuffer(GpuBuffer& Dest, const UploadBuffer& Src, size_t SrcOffset, size_t NumBytes /* = -1 */, size_t DestOffset /* = 0 */)
{

}

void CommandContext::InitializeBuffer(GpuBuffer& Dest, const void* Data, size_t NumBytes, size_t DestOffset /* = 0 */)
{

}

void CommandContext::TransitionResource(GPUResource& InResource, D3D12_RESOURCE_STATES NewState, bool FlushImmediate /* = false */)
{
	D3D12_RESOURCE_STATES OldState = InResource->GetResourceState();
	if (OldState != NewState)
	{
		ASSERT(Y_NumBarriesToFlush < 16, "Num Barriers to Flush exceed 16");

		D3D12_RESOURCE_BARRIER& ResourceBarrier = Y_ResourceBarrierBuffer[Y_NumBarriesToFlush++];
		ResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		
		ResourceBarrier.Transition.pResource = InResource->GetResource();
		ResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		ResourceBarrier.Transition.StateAfter = NewState;
		ResourceBarrier.Transition.StateBefore = OldState;

		if (NewState == InResource->GetResourceTransitionState())
		{
			ResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_END_ONLY;
			InResource->SetResourceTransitionState((D3D12_RESOURCE_STATES)-1);
		}
		else
		{
			ResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE
		}
		
		InResource->SetResourceState(NewState);

	}
	else if (NewState == D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
	{
		InsertUAVBarrier();
	}
	

	if (FlushImmediate || Y_NumBarriesToFlush == 16)
	{
		FlushResourceBarriers();
	}
}

void CommandContext::BeginResourceTransition(GPUResource& InResource, D3D12_RESOURCE_STATES NewState, bool FlushImmediate /* = false */)
{
	if (InResource->GetResourceTransitionState() !=(D3D12_RESOURCE_STATES)-1)
	{
		TransitionResource(InResource, InResource->GetResourceTransitionState());
	}

	D3D12_RESOURCE_STATES OldState = InResource->GetResourceState();
	if (OldState != NewState)
	{
		ASSERT(Y_NumBarriesToFlush < 16, "exceed arbitrary limit on buffered barries");
		D3D12_RESOURCE_BARRIER& ResourceBarrier = Y_ResourceBarrierBuffer[Y_NumBarriesToFlush++];
		ResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		ResourceBarrier.Transition.pResource = InResource->GetResource();
		ResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		ResourceBarrier.Transition.StateBefore = OldState;
		ResourceBarrier.Transition.StateAfter = NewState;

		ResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_BEGIN_ONLY;
		ResourceBarrier.SetResourceTransitionState(NewState);
	}

	if (FlushImmediate || Y_NumBarriesToFlush == 16)
	{
		FlushResourceBarriers();
	}
}

void CommandContext::InsertUAVBarrier(GPUResource& InResource, bool FlushImmediate)
{
	ASSERT(Y_NumBarriesToFlush < 16, "exceed arbitrary limit on buffered barries");
	D3D12_RESOURCE_BARRIER& ResourceBarrier = Y_ResourceBarrierBuffer[Y_NumBarriesToFlush++];

	ResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	ResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	ResourceBarrier.UAV.pResource = InResource->GetResource();

	if (FlushImmediate)
	{
		FlushResourceBarriers();
	}
}

void CommandContext::InsertAliasBarrier(GPUResource& Before, GPUResource& After, bool FlushImmediate)
{
	ASSERT(Y_NumBarriesToFlush < 16, "exceed arbitrary limit on buffered barries");
	D3D12_RESOURCE_BARRIER& ResourceBarrier = Y_ResourceBarrierBuffer[Y_NumBarriesToFlush++];

	ResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_ALIASING;
	ResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	ResourceBarrier.Aliasing.pResourceBefore = Before->GetResource();
	ResourceBarrier.Aliasing.pResourceAfter = Before->GetResource();

	if (FlushImmediate)
	{
		FlushResourceBarriers();
	}
}