#pragma once
#include "../Buffer/GPUBuffer.h"
#include "../Buffer/UploadBuffer.h"
#include "CommandListManager.h"
#include "PipelineState.h"
#include "../Buffer/LinearAllocator.h"

struct FDWParam
{
	FDWParam(FLOAT f):Float(f){}
	void operator= (FLOAT f) { Float = f; }

	FDWParam(UINT u) :Uint(u) {}
	void operator= (UINT u) { Uint = u; }

	FDWParam(INT i) :Int(i) {}
	void operator= (INT i) { Int = i; }

	union
	{
		FLOAT Float;
		UINT Uint;
		INT Int;
	};
};

class FContextManager
{
public:
	FContextManager(void) {}

	FCommandContext* RequestContext(D3D12_COMMAND_LIST_TYPE Type);
	void ReleaseContext(FCommandContext*);

	void DestroyAllContexts();

private:
	std::vector<std::unique_ptr<FCommandContext>> ContextPool[4];
	std::queue<FCommandContext*> AvailableContextPool[4];
	std::mutex ContextAvalibleMutex;
};

struct FNonCopyable
{
	FNonCopyable() = default;
	FNonCopyable(const FNonCopyable&) = delete;
	FNonCopyable& operator=(const FNonCopyable&) = delete;
};

class FGpuBuffer;
class FGraphicsContext;
class FCommandContext : public FNonCopyable
{
	friend class FContextManager;
private:

	FCommandContext(D3D12_COMMAND_LIST_TYPE Type);

	void Reset(void);

public:
	~FCommandContext(void);

	static void DestroyAllContexts(void);

	static FCommandContext& Begin(const std::wstring ID = L"");

	FGraphicsContext& GetGraphicsContext()
	{
		ASSERT(CommandType != D3D12_COMMAND_LIST_TYPE_COMPUTE, "Can not cast compute command list to graph commandlist");
		return reinterpret_cast<FGraphicsContext&>(*this);
	}

	void Initialize(void);

	//flush the exit commands to gpu ; but keep the context alive
	uint64_t Flush(bool WaitForCompletion = false);
	//flush the exit commands to gpu ; and release the current context
	uint64_t Finish(bool WaitForCompletion = false);

	//void CopyBuffer(FGPUResource& Desc, FGPUResource& Src);
	//void CopyBufferRegion(FGPUResource& Desc, size_t DestOffset, FGPUResource& Src, size_t SrcOffst, size_t NumBytes);
	//void CopySubResource(FGPUResource& Desc, UINT DestSubIndex, FGPUResource& Src, UINT SrcSubIndex);
	//void CopyCounter(FGPUResource& Desc, size_t DestOffset, FStructuredBuffer& Src);
	//void CopyTextureRegion(FGPUResource& Desc, UINT X, UINT Y, UINT Z, FGPUResource& Source, RECT& Rect);
	//void ResetCounter(FStructuredBuffer& Buf, uint32_t Value = 0);



	void TransitionResource(FGPUResource& InResource, D3D12_RESOURCE_STATES NewState, bool FlushImmediate = false);
	void BeginResourceTransition(FGPUResource& InResource, D3D12_RESOURCE_STATES NewState, bool FlushImmediate = false);
	void InsertUAVBarrier(FGPUResource& InResource, bool FlushImmediate);
	void InsertAliasBarrier(FGPUResource& Before, FGPUResource& After, bool FlushImmediate);
	
	inline void FlushResourceBarriers(void);

	FDynAlloc ReservedUploadMemory(size_t SizeInBytes)
	{
		return CpuLinearAllocator.Allocate(SizeInBytes);
	}

	static void InitializeTexture(FGPUResource& Dest, UINT NumSubResource, D3D12_SUBRESOURCE_DATA SubData[]);
	static void InitializeBuffer(FGpuBuffer& Dest, const void* Data, size_t NumBytes, size_t DestOffset = 0);
	static void InitializeBuffer(FGpuBuffer& Dest, const FUploadBuffer& Src, size_t SrcOffset, size_t NumBytes = -1, size_t DestOffset = 0);
	static void InitializeTextureArraySlice(FGPUResource& Dest, UINT SliceIndex, FGPUResource& Src);

	//void WriteBuffer(FGPUResource& Dest, size_t DestOffset, const void* Data, size_t NumBytes);
	//void FillBuffer(FGPUResource& Desc, size_t DestOffset, FDWParam Value, size_t NumBytes);

	//void InsertTimeStamp(ID3D12QueryHeap* QueryHeap, uint32_t QueryIdx);
	//void ResolveTimeStamps(ID3D12Resource* ReadbackHeap, ID3D12QueryHeap* QueryHeap, uint32_t NumQueris);
	//void PIXBeginEvent(const wchar_t* Label);
	//void PIXEndEvent(void);
	//void PIXSetMarker(const wchar_t* Label);

	//void SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE Type, ID3D12DescriptorHeap* HeapPtr);
	//void SetDescriptorHeaps(UINT HeapCount, D3D12_DESCRIPTOR_HEAP_TYPE Type[], ID3D12DescriptorHeap* HeapPtrs[]);
	//void SetPipelineState(const FPSO& Pso);

	//void SetPrediction(ID3D12Resource* Buffer, uint64_t BufferOffset, D3D12_PREDICATION_OP Op);
protected:

	void BuildDescriptorHeaps();

	FCommandListmanager* OwingCommandManager;
	ID3D12GraphicsCommandList* CommandList;
	ID3D12CommandAllocator* CommandAllocator;

	ID3D12RootSignature* GraphicsRootSignature;
	ID3D12RootSignature* ComputeRootSignature;
	ID3D12PipelineState* PipelineState;

	//dynamic descriptor heaps

	D3D12_RESOURCE_BARRIER ResourceBarrierBuffer[16];
	UINT NumBarriesToFlush;

	ID3D12DescriptorHeap* DescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];

	FLinearAllocator CpuLinearAllocator;
	FLinearAllocator GpuLinearAllocator;

	std::wstring ID;
	void SetID(const std::wstring& InID) { ID = InID; }

	D3D12_COMMAND_LIST_TYPE CommandType;

	//for buffer initialize


};


inline void FCommandContext::FlushResourceBarriers()
{
	if (NumBarriesToFlush >0)
	{
		CommandList->ResourceBarrier(NumBarriesToFlush, ResourceBarrierBuffer);
		NumBarriesToFlush = 0;
	}
}


class FGraphicsContext : public FCommandContext
{
public:
	//static GraphicsContext& Begin(const std::wstring ID = L"")
	//{
	//	return CommandContext::Begin(ID).GetGraphicsContext();
	//}



private:

};
