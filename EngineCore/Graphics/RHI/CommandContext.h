#pragma once
#include "../Buffer/GPUBuffer.h"
#include "../Buffer/UploadBuffer.h"
#include "CommandListManager.h"
#include "PipelineState.h"

struct DWParam
{
	DWParam(FLOAT f):Float(f){}
	void operator= (FLOAT f) { Float = f; }

	DWParam(UINT u) :Uint(u) {}
	void operator= (INT u) { Uint = u; }
	DWParam(INT i) :Int(i) {}
	void operator= (INT i) { Int = i; }

	union
	{
		FLOAT Float;
		UINT Uint;
		INT Int;
	};
};

class ContextManager
{
public:
	ContextManager(void) {}

	CommandContext* RequestContext(D3D12_COMMAND_LIST_TYPE Type);
	void ReleaseContext(CommandContext*);

	void DestroyAllContexts();

private:
	std::vector<std::unique_ptr<CommandContext>> Y_ContextPool[4];
	std::queue<std::unique_ptr<CommandContext>> Y_AvailableContextPool[4];
	std::mutex Y_ContextAvalibleMutex;
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

	void CopyBuffer(GPUResource& Desc, GPUResource& Src);
	void CopyBufferRegion(GPUResource& Desc, size_t DestOffset, GPUResource& Src, size_t SrcOffst, size_t NumBytes);
	void CopySubResource(GPUResource& Desc, UINT DestSubIndex, GPUResource& Src, UINT SrcSubIndex);
	void CopyCounter(GPUResource& Desc, size_t DestOffset, StructuredBuffer& Src);
	void CopyTextureRegion(GPUResource& Desc, UINT X, UINT Y, UINT Z, GPUResource& Source, RECT& Rect);
	void ResetCounter(StructuredBuffer& Buf, uint32_t Value = 0);



	void TransitionResource(GPUResource& Resource, D3D12_RESOURCE_STATES NewState, bool FlushImmediate = false);
	void BeginResourceTransition(GPUResource& Resource, D3D12_RESOURCE_STATES NewState, bool FlushImmediate = false);
	void InsertUAVBarrier();
	void InsertAliasBarrier();
	
	inline void FlushResourceBarriers(void);

	static void InitializeTexture(GPUResource& Desc, UINT NumSubResource, D3D12_SUBRESOURCE_DATA SubData[]);
	static void InitializeBuffer(GpuBuffer& Dest, const void* Data, size_t NumBytes, size_t DestOffset = 0);
	static void InitializeBuffer(GpuBuffer& Dest, const UploadBuffer& Src, size_t SrcOffset, size_t NumBytes = -1, size_t DestOffset = 0);
	static void InitializeTextureArraySlice(GPUResource& Desc, UINT SliceIndec, GPUResource& Src);

	void WriteBuffer(GPUResource& Dest, size_t DestOffset, const void* Data, size_t NumBytes);
	void FillBuffer(GPUResource& Desc, size_t DestOffset, DWParam Value, size_t NumBytes);

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

	CommandListmanager* Y_OwingCommandManager;
	ID3D12GraphicsCommandList* Y_CommandList;
	ID3D12CommandAllocator* Y_CommandAllocator;

	ID3D12RootSignature* Y_GraphicsRootSignature;
	ID3D12RootSignature* Y_ComputeRootSignature;
	ID3D12PipelineState* Y_PipelineState;

	//dynamic descriptor heaps

	D3D12_RESOURCE_BARRIER Y_ResourceBarrierBuffer[16];
	UINT Y_NumBarriesToFlush;

	ID3D12DescriptorHeap* Y_DescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];

	D3D12_COMMAND_LIST_TYPE Y_CommandType;

};

inline void CommandContext::FlushResourceBarriers()
{
	if (Y_NumBarriesToFlush >0)
	{
		Y_CommandList->ResourceBarrier(Y_NumBarriesToFlush, Y_ResourceBarrierBuffer);
		Y_NumBarriesToFlush = 0;
	}
}


class GraphicsContext : public CommandContext
{
public:
	static GraphicsContext& Begin(const std::wstring ID = L"")
	{
		return CommandContext::Begin(ID).GetGraphicsContext();
	}



private:

};
