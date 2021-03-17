#pragma once
#include "../Buffer/GPUBuffer.h"
#include "../Buffer/UploadBuffer.h"
#include "CommandListManager.h"
#include "PipelineState.h"

class ContextManager
{
public:
	ContextManager(void) {}

	CommandContext* RequestContext(D3D12_COMMAND_LIST_TYPE Type);
	void ReleaseContext(CommandContext*);

	void DestroyAllContexts();

private:
	std::vector<std::unique_ptr<CommandContext>> ContextPool[4];
	std::queue<std::unique_ptr<CommandContext>> AvailableContextPool[4];
	std::mutex ContextAvalibleMutex;
};


struct NonCopyable
{
	NonCopyable() = default;
	NonCopyable(const NonCopyable&) = delete;
	NonCopyable& operator=(const NonCopyable&) = delete;
};

class CommandContext : public NonCopyable
{
	friend ContextManager;
private:

	CommandContext(D3D12_COMMAND_LIST_TYPE Type);

	void Reset(void);

public:
	~CommandContext(void);

	static void DestroyAllContexts(void);

	static CommandContext& Begin(const std::wstring ID = L"");

	GraphicsContext& GetGraphicsContext()
	{
		ASSERT(CommandType != D3D12_COMMAND_LIST_TYPE_COMPUTE, "Can not cast compute command list to graph commandlist");
		return reinterpret_cast<GraphicsContext&>(*this);
	}

	void Initialize(void);

	//flush the exit commands to gpu ; but keep the context alive
	uint64_t Flush(bool WaitForCompletion = false);
	//flush the exit commands to gpu ; and release the current context
	uint64_t Finish(bool WaitForCompletion = false);

	void TransitionResource(GPUResource& Resource, D3D12_RESOURCE_STATES NewState, bool FlushImmediate = false);
	void BeginResourceTransilation(GPUResource& Resource, D3D12_RESOURCE_STATES NewState, bool FlushImmediate = false);

	static void InitializeBuffer(GpuBuffer& Dest, const void* Data, size_t NumBytes, size_t DestOffset = 0);
	static void InitializeBuffer(GpuBuffer& Dest, const UploadBuffer& Src, size_t SrcOffset, size_t NumBytes = -1, size_t DestOffset = 0);

	void InsertTimeStamp(ID3D12QueryHeap* QueryHeap, uint32_t QueryIdx);
	void ResolveTimeStamps(ID3D12Resource* ReadbackHeap, ID3D12QueryHeap* QueryHeap, uint32_t NumQueris);
	void PIXBeginEvent(const wchar_t* Label);
	void PIXEndEvent(void);
	void PIXSetMarker(const wchar_t* Label);

	void SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE Type, ID3D12DescriptorHeap* HeapPtr);
	void SetDescriptorHeaps(UINT HeapCount, D3D12_DESCRIPTOR_HEAP_TYPE Type[], ID3D12DescriptorHeap* HeapPtrs[]);
	void SetPipelineState(const PSO& Pso);

	void SetPrediction(ID3D12Resource* Buffer, uint64 BufferOffset, D3D12_PREDICATION_OP Op);
protected:

	void BuildDescriptorHeaps();

	CommandListmanager* OwingCommandManager;
	ID3D12GraphicsCommandList* CommandList;
	ID3D12CommandAllocator* CommandAllocator;

	ID3D12RootSignature* GraphicsRootSignature;
	ID3D12RootSignature* ComputeRootSignature;
	ID3D12PipelineState* PipelineState;

	D3D12_RESOURCE_BARRIER ResourceBarrierBuffer[16];
	UINT NumBarriesToFlush;

	ID3D12DescriptorHeap* DescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];

	D3D12_COMMAND_LIST_TYPE CommandType;

};


class GraphicsContext : public CommandContext
{
public:
	static GraphicsContext& Begin(const std::wstring ID = L"")
	{
		return CommandContext::Begin(ID).GetGraphicsContext();
	}



private:

};
