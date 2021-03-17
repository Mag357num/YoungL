#include "../../pch.h"
#include "CommandContext.h"

CommandContext* ContextManager::RequestContext(D3D12_COMMAND_LIST_TYPE Type)
{
	std::lock_guard<std::mutex> LockGuard(ContextAvalibleMutex);
	CommandContext* Ret = nullptr;

	auto& AvailableComamndContext = AvailableContextPool[Type];
	if (AvailableComamndContext.empty())
	{
		Ret = new CommandContext(Type);
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

void ContextManager::ReleaseContext(CommandContext* UsedContext)
{
	ASSERT(UsedContext != nullptr);
	std::lock_guard<std::mutex> LockGuard(ContextAvalibleMutex);
	AvailableContextPool[UsedContext->CommandType].push(UsedContext);
}

void ContextManager::DestroyAllContexts()
{
	for (uint32_t i=0; i< 4; i++)
	{
		ContextPool[i].clear();
	}
}

CommandContext::CommandContext(D3D12_COMMAND_LIST_TYPE Type)
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

CommandContext::~CommandContext(void)
{
	if (CommandList != nullptr)
	{
		CommandList->Release();
	}
}


void CommandContext::InitializeBuffer(GpuBuffer& Dest, const UploadBuffer& Src, size_t SrcOffset, size_t NumBytes /* = -1 */, size_t DestOffset /* = 0 */)
{

}

void CommandContext::InitializeBuffer(GpuBuffer& Dest, const void* Data, size_t NumBytes, size_t DestOffset /* = 0 */)
{

}

void CommandContext::TransitionResource(GPUResource& Resource, D3D12_RESOURCE_STATES NewState, bool FlushImmediate /* = false */)
{

}