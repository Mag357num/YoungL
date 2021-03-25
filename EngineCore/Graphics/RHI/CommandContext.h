#pragma once
#include "../Buffer/GPUBuffer.h"
#include "../Buffer/UploadBuffer.h"
#include "CommandListManager.h"
#include "PipelineState.h"
#include "../Buffer/LinearAllocator.h"
#include "DynamicDescriptorHeap.h"

#include "../../Math/Common.h"

//#include "../Buffer/ColorBuffer.h"
//#include "../Buffer/DepthBuffer.h"
#include "../Buffer/Color.h"

class FGpuBuffer;
class FColorBuffer;
class FDepthBuffer;
class FColor;
class FCommandSignature;

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

	void CopyBuffer(FGPUResource& Dest, FGPUResource& Src);
	void CopyBufferRegion(FGPUResource& Dest, size_t DestOffset, FGPUResource& Src, size_t SrcOffst, size_t NumBytes);
	void CopySubResource(FGPUResource& Dest, UINT DestSubIndex, FGPUResource& Src, UINT SrcSubIndex);
	void CopyCounter(FGPUResource& Dest, size_t DestOffset, FStructuredBuffer& Src);
	void CopyTextureRegion(FGPUResource& Dest, UINT X, UINT Y, UINT Z, FGPUResource& Source, RECT& Rect);
	void ResetCounter(FStructuredBuffer& Buf, uint32_t Value = 0);



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

	void WriteBuffer(FGPUResource& Dest, size_t DestOffset, const void* Data, size_t NumBytes);
	void FillBuffer(FGPUResource& Dest, size_t DestOffset, FDWParam Value, size_t NumBytes);

	void InsertTimeStamp(ID3D12QueryHeap* QueryHeap, uint32_t QueryIdx);
	void ResolveTimeStamps(ID3D12Resource* ReadbackHeap, ID3D12QueryHeap* QueryHeap, uint32_t NumQueris);
	
	void PIXBeginEvent(const wchar_t* Label);
	void PIXEndEvent(void);
	void PIXSetMarker(const wchar_t* Label);

	void SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE Type, ID3D12DescriptorHeap* HeapPtr);
	void SetDescriptorHeaps(UINT HeapCount, D3D12_DESCRIPTOR_HEAP_TYPE Type[], ID3D12DescriptorHeap* HeapPtrs[]);
	void SetPipelineState(const FPSO& Pso);

	void SetPrediction(ID3D12Resource* Buffer, uint64_t BufferOffset, D3D12_PREDICATION_OP Op);
protected:

	void BuildDescriptorHeaps();

	FCommandListmanager* OwingCommandManager;
	ID3D12GraphicsCommandList* CommandList;
	ID3D12CommandAllocator* CommandAllocator;

	ID3D12RootSignature* GraphicsRootSignature;
	ID3D12RootSignature* ComputeRootSignature;
	ID3D12PipelineState* PipelineState;

	//dynamic descriptor heaps
	FDynamicDescriptorHeap DynamicViewDescriptorHeap;
	FDynamicDescriptorHeap DynamicSamplerDescriptorHeap;


	D3D12_RESOURCE_BARRIER ResourceBarrierBuffer[16];
	UINT NumBarriesToFlush;

	ID3D12DescriptorHeap* DescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];

	//for buffer initialize
	FLinearAllocator CpuLinearAllocator;
	FLinearAllocator GpuLinearAllocator;

	std::wstring ID;
	void SetID(const std::wstring& InID) { ID = InID; }

	D3D12_COMMAND_LIST_TYPE CommandType;

	


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
	static FGraphicsContext& Begin(const std::wstring ID = L"")
	{
		return FCommandContext::Begin(ID).GetGraphicsContext();
	}

	void ClearUAV(FGpuBuffer& Target);
	void ClearUAV(FColorBuffer& Target);
	void ClearColor(FColorBuffer& Target, D3D12_RECT* Rect = nullptr);
	void ClearColor(FColorBuffer& Target, float Colour[4], D3D12_RECT* Rect = nullptr);
	void ClearDepth(FDepthBuffer& Target);
	void ClearStencil(FDepthBuffer& Target);
	void ClearDepthAndStencil(FDepthBuffer& Target);

	void BeignQuery(ID3D12QueryHeap* QueryHeap, D3D12_QUERY_TYPE InType, UINT HeapIndex);
	void EndQuery(ID3D12QueryHeap* QueryHeap, D3D12_QUERY_TYPE InType, UINT HeapIndex);
	void ResolveQueryData(ID3D12QueryHeap* QueryHeap, D3D12_QUERY_TYPE InType, UINT StartIndex, UINT NumQueries,
		ID3D12Resource* DestinationBuffer, UINT64 DesctinationBufferOffset);

	void SetRenderTargets(UINT NumRtvs, const D3D12_CPU_DESCRIPTOR_HANDLE Rtvs[]);
	void SetRenderTargets(UINT NumRtvs, const D3D12_CPU_DESCRIPTOR_HANDLE Rtvs[], D3D12_CPU_DESCRIPTOR_HANDLE Dsv);
	void SetRenderTargets(D3D12_CPU_DESCRIPTOR_HANDLE Rtv) { SetRenderTargets(1, &Rtv); }
	void SetRenderTargets(D3D12_CPU_DESCRIPTOR_HANDLE Rtv, D3D12_CPU_DESCRIPTOR_HANDLE Dsv) { SetRenderTargets(1, &Rtv, Dsv); }
	void SetDepthStendilTarget(D3D12_CPU_DESCRIPTOR_HANDLE Dsv) { SetRenderTargets(0, nullptr, Dsv); }

	void SetViewport(const D3D12_VIEWPORT& Vp);
	void SetViewport(FLOAT X, FLOAT Y, FLOAT W, FLOAT H, FLOAT MinDepth = 0.0f, FLOAT MaxDepth = 1.0f);
	void SetScissor(const D3D12_RECT& Rect);
	void SetScissor(UINT Left, UINT Top, UINT Right, UINT Bottom);
	void SetViewportAndScissor(const D3D12_VIEWPORT& Vp, const D3D12_RECT& rect);
	void SetViewportAndScissor(UINT X, UINT Y, UINT W, UINT H);
	void SetStencilRef(UINT StencilRef);
	void SetBlendFactor(FColor BlendFactor);
	void SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY InTopology);

	void SetConstantArray(UINT RootIndex, UINT NumConstants, const void* ContantsData);
	void SetConstant(UINT RootEntry, UINT Offset, FDWParam Val);
	void SetConstants(UINT RootIndex, FDWParam X);
	void SetConstants(UINT RootIndex, FDWParam X, FDWParam Y);
	void SetConstants(UINT RootIndex, FDWParam X, FDWParam Y, FDWParam Z);
	void SetConstants(UINT RootIndex, FDWParam X, FDWParam Y, FDWParam Z, FDWParam W);
	void SetConstantBuffer(UINT RootIndex, D3D12_GPU_VIRTUAL_ADDRESS Cbv);
	void SetDynamicConstantBufferView(UINT RootIndex, size_t BufferSize, const void* BufferData);
	void SetBufferSrv(UINT RootIndex, FGpuBuffer& InSrv, UINT64 Offset = 0);
	void SetBufferUav(UINT RootIndex, FGpuBuffer& Uav, UINT64 Offset = 0);
	void SetDescriptorTable(UINT RootIndex, D3D12_GPU_DESCRIPTOR_HANDLE FirstHandle);

	void SetDynamicDescriptor(UINT RootIndex, UINT Offset, D3D12_CPU_DESCRIPTOR_HANDLE Handle);
	void SetDynamicDescriptors(UINT RootIndex, UINT Offset, UINT Count, const D3D12_CPU_DESCRIPTOR_HANDLE Handles[]);
	void SetDynamicSampler(UINT RootIndex, UINT Offset, D3D12_CPU_DESCRIPTOR_HANDLE Handle);
	void SetDynamicSamplers(UINT RootIndex, UINT Offset, UINT Count, const D3D12_CPU_DESCRIPTOR_HANDLE Handles[]);

	void SetIndexBuffer(const D3D12_INDEX_BUFFER_VIEW& IbView);
	void SetVertexBuffer(UINT Slot, const D3D12_VERTEX_BUFFER_VIEW& VbView);
	void SetVertexBuffers(UINT StartSlot, UINT Count, const D3D12_VERTEX_BUFFER_VIEW VbViews[]);
	void SetDynamicVB(UINT Slot, size_t NumVertices, size_t VertexStride, const void* VBData);
	void SetDynamicIB(size_t IndexCount, const uint16_t* IBData);
	void SetDynamicSrv(UINT RootIndex, size_t BufferSize, const void* BufferData);

	void Draw(UINT VertexCount, UINT VertexStartOffset = 0);
	void DrawIndexed(UINT IndexCount, UINT StartIndexLocation = 0, UINT BaseVertexLocation = 0);
	void DrawInstanced(UINT VertexCountPerInstance, UINT InstanceCount,
		UINT StartVertexLocation = 0, UINT StartIndexLocation = 0);
	void DrawIndexedInstanced(UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation,
		INT BaseVertexLocation, UINT StartInstanceLocation);

	void DrawIndirect(FGpuBuffer& ArgumentBuffer, uint64_t ArgumentBufferOffset = 0);
	void ExcuteIndirect(FCommandSignature& CommandSig, FGpuBuffer& ArgumentBuffer, uint64_t ArgumentStartOffset = 0,
		uint32_t MaxComamnds = 1, FGpuBuffer* CommandCounterBuffer = nullptr, uint64_t CounterOffset = 0);

private:

};
