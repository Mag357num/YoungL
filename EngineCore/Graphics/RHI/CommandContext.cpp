#include "../../pch.h"
#include "CommandContext.h"
#include "../GraphicsCore.h"
#include "../Buffer/GPUResource.h"

using namespace Graphics;

FCommandContext* FContextManager::RequestContext(D3D12_COMMAND_LIST_TYPE Type)
{
	std::lock_guard<std::mutex> LockGuard(ContextAvalibleMutex);
	FCommandContext* Ret = nullptr;

	auto& AvailableComamndContext = AvailableContextPool[Type];
	if (AvailableComamndContext.empty())
	{
		Ret = new FCommandContext(Type);
		ContextPool[Type].emplace_back(Ret);
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

void FContextManager::ReleaseContext(FCommandContext* UsedContext)
{
	ASSERT(UsedContext != nullptr);
	std::lock_guard<std::mutex> LockGuard(ContextAvalibleMutex);
	AvailableContextPool[UsedContext->CommandType].push(UsedContext);
}

void FContextManager::DestroyAllContexts()
{
	for (uint32_t i=0; i< 4; i++)
	{
		ContextPool[i].clear();
	}
}

FCommandContext::FCommandContext(D3D12_COMMAND_LIST_TYPE Type)
	:CommandType(Type)
{
	OwingCommandManager = nullptr;
	CommandAllocator = nullptr;
	CommandList = nullptr;
	ZeroMemory(DescriptorHeaps, sizeof(DescriptorHeaps));

	GraphicsRootSignature = nullptr;
	ComputeRootSignature = nullptr;
	PipelineState = nullptr;
	NumBarriesToFlush = 0;
}

FCommandContext::~FCommandContext(void)
{
	if (CommandList != nullptr)
	{
		CommandList->Release();
	}
}

void FCommandContext::Initialize()
{
	g_CommandManager.CreateNewCommandList(CommandType, &CommandList, &CommandAllocator);
}

void FCommandContext::Reset()
{
	ASSERT(CommandList != nullptr && CommandAllocator == nullptr);
	CommandAllocator = g_CommandManager.GetQueue(CommandType).RequestAllocator();
	CommandList->Reset(CommandAllocator, nullptr);

	GraphicsRootSignature = nullptr;
	ComputeRootSignature = nullptr;
	PipelineState = nullptr;
	NumBarriesToFlush = 0;

	BuildDescriptorHeaps();
}


void FCommandContext::BuildDescriptorHeaps()
{
	UINT NonHeaps = 0;
	ID3D12DescriptorHeap* HeapToBind[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
	for (UINT i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; i++)
	{
		ID3D12DescriptorHeap* Heap = DescriptorHeaps[i];
		if (Heap != nullptr)
		{
			HeapToBind[NonHeaps++] = Heap;
		}
	}

	CommandList->SetDescriptorHeaps(NonHeaps, HeapToBind);
}

uint64_t FCommandContext::Flush(bool WaitForCompletion /* = false */)
{
	FlushResourceBarriers();
	ASSERT(CommandAllocator != nullptr);

	uint64_t FenceValue = g_CommandManager.GetQueue(CommandType).ExcuteCommandList(CommandList);
	if (WaitForCompletion)
	{
		g_CommandManager.WaitForFence(FenceValue);
	}

	CommandList->Reset(CommandAllocator, nullptr);
	if (GraphicsRootSignature)
	{
		CommandList->SetGraphicsRootSignature(GraphicsRootSignature);
	}

	if (ComputeRootSignature)
	{
		CommandList->SetComputeRootSignature(ComputeRootSignature);
	}

	if (PipelineState)
	{
		CommandList->SetPipelineState(PipelineState);
	}

	BuildDescriptorHeaps();
	return FenceValue;

}

uint64_t FCommandContext::Finish(bool WaitForCompletion /* = false */)
{
	ASSERT(CommandType == D3D12_COMMAND_LIST_TYPE_DIRECT | D3D12_COMMAND_LIST_TYPE_COMPUTE);
	FlushResourceBarriers();

	FCommandQueue& CmdQueue = g_CommandManager.GetQueue(CommandType);
	uint64_t FenceValue = CmdQueue.ExcuteCommandList(CommandList);
	CmdQueue.DiscardAllocator(FenceValue, CommandAllocator);
	CommandAllocator = nullptr;

	if (WaitForCompletion)
	{
		g_CommandManager.WaitForFence(FenceValue);
	}
	g_ContextManager.ReleaseContext(this);

	return FenceValue;
}

void FCommandContext::DestroyAllContexts()
{
	g_ContextManager.DestroyAllContexts();
}


void FCommandContext::InitializeTexture(FGPUResource& Desc, UINT NumSubResource, D3D12_SUBRESOURCE_DATA SubData[])
{

}

void FCommandContext::InitializeTextureArraySlice(FGPUResource& Desc, UINT SliceIndec, FGPUResource& Src)
{

}

void FCommandContext::InitializeBuffer(FGpuBuffer& Dest, const FUploadBuffer& Src, size_t SrcOffset, size_t NumBytes /* = -1 */, size_t DestOffset /* = 0 */)
{
	//FCommandContext& Context = FCommandContext::Begin();

}

void FCommandContext::InitializeBuffer(FGpuBuffer& Dest, const void* Data, size_t NumBytes, size_t DestOffset /* = 0 */)
{

}

void FCommandContext::TransitionResource(FGPUResource& InResource, D3D12_RESOURCE_STATES NewState, bool FlushImmediate /* = false */)
{
	D3D12_RESOURCE_STATES OldState = InResource.GetResourceState();
	if (OldState != NewState)
	{
		ASSERT(NumBarriesToFlush < 16, "Num Barriers to Flush exceed 16");

		D3D12_RESOURCE_BARRIER& ResourceBarrier = ResourceBarrierBuffer[NumBarriesToFlush++];
		ResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		
		ResourceBarrier.Transition.pResource = InResource.GetResource();
		ResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		ResourceBarrier.Transition.StateAfter = NewState;
		ResourceBarrier.Transition.StateBefore = OldState;

		if (NewState == InResource.GetResourceTransitionState())
		{
			ResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_END_ONLY;
			InResource.SetResourceTransitionState((D3D12_RESOURCE_STATES)-1);
		}
		else
		{
			ResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		}
		
		InResource.SetResourceState(NewState);

	}
	else if (NewState == D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
	{
		InsertUAVBarrier(InResource, FlushImmediate);
	}
	

	if (FlushImmediate || NumBarriesToFlush == 16)
	{
		FlushResourceBarriers();
	}
}

void FCommandContext::BeginResourceTransition(FGPUResource& InResource, D3D12_RESOURCE_STATES NewState, bool FlushImmediate /* = false */)
{
	if (InResource.GetResourceTransitionState() !=(D3D12_RESOURCE_STATES)-1)
	{
		TransitionResource(InResource, InResource.GetResourceTransitionState());
	}

	D3D12_RESOURCE_STATES OldState = InResource.GetResourceState();
	if (OldState != NewState)
	{
		ASSERT(NumBarriesToFlush < 16, "exceed arbitrary limit on buffered barries");
		D3D12_RESOURCE_BARRIER& ResourceBarrier = ResourceBarrierBuffer[NumBarriesToFlush++];
		ResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		ResourceBarrier.Transition.pResource = InResource.GetResource();
		ResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		ResourceBarrier.Transition.StateBefore = OldState;
		ResourceBarrier.Transition.StateAfter = NewState;

		ResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_BEGIN_ONLY;
		InResource.SetResourceTransitionState(NewState);
	}

	if (FlushImmediate || NumBarriesToFlush == 16)
	{
		FlushResourceBarriers();
	}
}

void FCommandContext::InsertUAVBarrier(FGPUResource& InResource, bool FlushImmediate)
{
	ASSERT(NumBarriesToFlush < 16, "exceed arbitrary limit on buffered barries");
	D3D12_RESOURCE_BARRIER& ResourceBarrier = ResourceBarrierBuffer[NumBarriesToFlush++];

	ResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	ResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	ResourceBarrier.UAV.pResource = InResource.GetResource();

	if (FlushImmediate)
	{
		FlushResourceBarriers();
	}
}

void FCommandContext::InsertAliasBarrier(FGPUResource& Before, FGPUResource& After, bool FlushImmediate)
{
	ASSERT(NumBarriesToFlush < 16, "exceed arbitrary limit on buffered barries");
	D3D12_RESOURCE_BARRIER& ResourceBarrier = ResourceBarrierBuffer[NumBarriesToFlush++];

	ResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_ALIASING;
	ResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	ResourceBarrier.Aliasing.pResourceBefore = Before.GetResource();
	ResourceBarrier.Aliasing.pResourceAfter = After.GetResource();

	if (FlushImmediate)
	{
		FlushResourceBarriers();
	}
}