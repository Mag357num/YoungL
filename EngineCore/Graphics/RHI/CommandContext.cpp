#include "pch.h"
#include "CommandContext.h"
#include "../GraphicsCore.h"
#include "../Buffer/GPUResource.h"
#include "../../Math/Common.h"

#include "../Buffer/ColorBuffer.h"
#include "../Buffer/DepthBuffer.h"
#include "CommandSignature.h"

#pragma warning(push)
#pragma warning(disable:4100)
#include <pix.h>
#pragma wanrning(pop);

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
	:CommandType(Type),
	CpuLinearAllocator(ECpuWriteable),
	GpuLinearAllocator(EGpuExlusive),
	DynamicViewDescriptorHeap(*this, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV),
	DynamicSamplerDescriptorHeap(*this, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER)

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

FCommandContext& FCommandContext::Begin(const std::wstring ID /* = L"" */)
{
	FCommandContext* NewContext = g_ContextManager.RequestContext(D3D12_COMMAND_LIST_TYPE_DIRECT);
	NewContext->SetID(ID);
	//if (ID.length() > 0)
	//{
	//	EngineProfiling
	//}

	return *NewContext;
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


void FCommandContext::InitializeTexture(FGPUResource& Dest, UINT NumSubResource, D3D12_SUBRESOURCE_DATA SubData[])
{
	UINT64 UploadBufferSize = GetRequiredIntermediateSize(Dest.GetResource(), 0, NumSubResource);
	
	FCommandContext& Context = FCommandContext::Begin();
	FDynAlloc Mem = Context.ReservedUploadMemory(UploadBufferSize);
	UpdateSubresources(Context.CommandList, Dest.GetResource(), Mem.Buffer.GetResource(), 0, 0, NumSubResource, SubData);

	Context.TransitionResource(Dest, D3D12_RESOURCE_STATE_GENERIC_READ);

	Context.Finish(true);
}

void FCommandContext::InitializeTextureArraySlice(FGPUResource& Dest, UINT SliceIndex, FGPUResource& Src)
{
	FCommandContext& Context = FCommandContext::Begin();
	Context.TransitionResource(Dest, D3D12_RESOURCE_STATE_COPY_DEST);

	Context.FlushResourceBarriers();

	const D3D12_RESOURCE_DESC& DestDesc = Dest.GetResource()->GetDesc();
	const D3D12_RESOURCE_DESC& SrcDesc = Src.GetResource()->GetDesc();

	ASSERT(SliceIndex < DestDesc.DepthOrArraySize &&
			SrcDesc.DepthOrArraySize == 1&&
		DestDesc.Width == SrcDesc.Width&&
		DestDesc.Height == SrcDesc.Height &&
		DestDesc.MipLevels <= SrcDesc.MipLevels

	);

	UINT SubResourceIndex = SliceIndex * DestDesc.MipLevels;
	for (UINT i = 0; i < DestDesc.MipLevels; i++)
	{
		D3D12_TEXTURE_COPY_LOCATION DestCopyLoc =
		{
			Dest.GetResource(),
			D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX,
			SubResourceIndex + i
		};

		D3D12_TEXTURE_COPY_LOCATION SrcCopyLoc =
		{
			Src.GetResource(),
			D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX,
			i
		};
		Context.CommandList->CopyTextureRegion(&DestCopyLoc, 0, 0, 0, &SrcCopyLoc, nullptr);

	}

	Context.TransitionResource(Dest, D3D12_RESOURCE_STATE_GENERIC_READ);
	Context.Finish(true);
}

void FCommandContext::InitializeBuffer(FGpuBuffer& Dest, const FUploadBuffer& Src, size_t SrcOffset, size_t NumBytes /* = -1 */, size_t DestOffset /* = 0 */)
{
	FCommandContext& Context = FCommandContext::Begin();
	
	size_t MaxBytes = std::min<size_t>(Dest.GetBufferSize() - DestOffset, Src.GetBufferSize() - SrcOffset);
	NumBytes = std::min<size_t>(MaxBytes, NumBytes);

	Context.TransitionResource(Dest, D3D12_RESOURCE_STATE_COPY_DEST, true);
	Context.CommandList->CopyBufferRegion(Dest.GetResource(), DestOffset, (ID3D12Resource*)Src.GetResource(), SrcOffset, NumBytes);
	Context.TransitionResource(Dest, D3D12_RESOURCE_STATE_GENERIC_READ, true);

	Context.Finish(true);

}

void FCommandContext::InitializeBuffer(FGpuBuffer& Dest, const void* Data, size_t NumBytes, size_t DestOffset /* = 0 */)
{
	FCommandContext& Context = FCommandContext::Begin();
	FDynAlloc Mem = Context.ReservedUploadMemory(NumBytes);
	SIMDMemCopy(Mem.DataPtr, Data, Math::DivideByMultiple(NumBytes, 16));

	Context.TransitionResource(Dest, D3D12_RESOURCE_STATE_COPY_DEST, true);
	Context.CommandList->CopyBufferRegion(Dest.GetResource(), DestOffset, Mem.Buffer.GetResource(), 0, NumBytes);
	Context.TransitionResource(Dest, D3D12_RESOURCE_STATE_GENERIC_READ, true);

	Context.Finish(true);
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

void FCommandContext::CopyBuffer(FGPUResource& Dest, FGPUResource& Src)
{
	TransitionResource(Dest, D3D12_RESOURCE_STATE_COPY_DEST);
	TransitionResource(Src, D3D12_RESOURCE_STATE_COPY_SOURCE);
	FlushResourceBarriers();

	CommandList->CopyResource(Dest.GetResource(), Src.GetResource());
}

void FCommandContext::CopyBufferRegion(FGPUResource& Dest, size_t DestOffset, FGPUResource& Src, size_t SrcOffst, size_t NumBytes)
{
	TransitionResource(Dest, D3D12_RESOURCE_STATE_COPY_DEST);
	FlushResourceBarriers();

	CommandList->CopyBufferRegion(Dest.GetResource(), DestOffset, Src.GetResource(), SrcOffst, NumBytes);
}

void FCommandContext::CopyCounter(FGPUResource& Dest, size_t DestOffset, FStructuredBuffer& Src)
{
	TransitionResource(Dest, D3D12_RESOURCE_STATE_COPY_DEST);
	TransitionResource(Src.GetCounterBuffer(), D3D12_RESOURCE_STATE_COPY_SOURCE);
	FlushResourceBarriers();

	CommandList->CopyBufferRegion(Dest.GetResource(), DestOffset, Src.GetCounterBuffer().GetResource(), 0, 4);
}

void FCommandContext::CopySubResource(FGPUResource& Dest, UINT DestSubIndex, FGPUResource& Src, UINT SrcSubIndex)
{
	FlushResourceBarriers();

	D3D12_TEXTURE_COPY_LOCATION DestLoc =
	{
		Dest.GetResource(),
		D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX,
		DestSubIndex
	};

	D3D12_TEXTURE_COPY_LOCATION SrcLoc =
	{
		Src.GetResource(),
		D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX,
		SrcSubIndex
	};

	CommandList->CopyTextureRegion(&DestLoc, 0, 0, 0, &SrcLoc, nullptr);
}

void FCommandContext::CopyTextureRegion(FGPUResource& Dest, UINT X, UINT Y, UINT Z, FGPUResource& Source, RECT& Rect)
{
	D3D12_TEXTURE_COPY_LOCATION DestLoc = CD3DX12_TEXTURE_COPY_LOCATION(Dest.GetResource(), 0);
	D3D12_TEXTURE_COPY_LOCATION SrcLoc = CD3DX12_TEXTURE_COPY_LOCATION(Source.GetResource(), 0);

	D3D12_BOX Box = {};
	Box.back = 1;
	Box.left = Rect.left;
	Box.right = Rect.right;
	Box.top = Rect.top;
	Box.bottom = Rect.bottom;

	CommandList->CopyTextureRegion(&DestLoc, X, Y, X, &SrcLoc, &Box);
}

void FCommandContext::WriteBuffer(FGPUResource& Dest, size_t DestOffset, const void* Data, size_t NumBytes)
{
	ASSERT(Data != nullptr && Math::IsAligned(Data, 16));
	FDynAlloc TempSpace = CpuLinearAllocator.Allocate(NumBytes, 512);
	SIMDMemCopy(TempSpace.DataPtr, Data, Math::DivideByMultiple(NumBytes, 16));
	CopyBufferRegion(Dest, DestOffset, TempSpace.Buffer, TempSpace.Offset, NumBytes);
}

void FCommandContext::FillBuffer(FGPUResource& Dest, size_t DestOffset, FDWParam Value, size_t NumBytes)
{
	FDynAlloc TempSpace = CpuLinearAllocator.Allocate(NumBytes, 512);
	__m128 VectorValue = _mm_set1_ps(Value.Float);

	SIMDMemFill(TempSpace.DataPtr, VectorValue, Math::DivideByMultiple(NumBytes, 16));
	CopyBufferRegion(Dest, DestOffset, TempSpace.Buffer, TempSpace.Offset, NumBytes);
}

void FCommandContext::ResetCounter(FStructuredBuffer& Buf, uint32_t Value /* = 0 */)
{
	FillBuffer(Buf.GetCounterBuffer(), 0, Value, sizeof(uint32_t));
	TransitionResource(Buf.GetCounterBuffer(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
}


void FCommandContext::InsertTimeStamp(ID3D12QueryHeap* QueryHeap, uint32_t QueryIdx)
{
	CommandList->EndQuery(QueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, QueryIdx);
}

void FCommandContext::ResolveTimeStamps(ID3D12Resource* ReadbackHeap, ID3D12QueryHeap* QueryHeap, uint32_t NumQueris)
{
	CommandList->ResolveQueryData(QueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, 0, NumQueris, ReadbackHeap, 0);
}

void FCommandContext::PIXBeginEvent(const wchar_t* Label)
{
#ifndef RELEASE
	::PIXBeginEvent(CommandList, 0, Label);
#endif // !RELEASE
}

void FCommandContext::PIXEndEvent()
{
#ifndef RELEASE
	::PIXEndEvent(CommandList);
#endif // !RELEASE
}

void FCommandContext::PIXSetMarker(const wchar_t* Label)
{
#ifndef RELEASE
	::PIXSetMarker(CommandList, 0, Label);
#endif // !RELEASE
}

void FCommandContext::SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE Type, ID3D12DescriptorHeap* HeapPtr)
{
	if (DescriptorHeaps[Type] != HeapPtr)
	{
		DescriptorHeaps[Type] = HeapPtr;
		BuildDescriptorHeaps();
	}
}

void FCommandContext::SetDescriptorHeaps(UINT HeapCount, D3D12_DESCRIPTOR_HEAP_TYPE Type[], ID3D12DescriptorHeap* HeapPtrs[])
{
	bool AnyChanged = false;

	for (UINT i =0; i< HeapCount; ++i)
	{
		if (DescriptorHeaps[Type[i]] != HeapPtrs[i])
		{
			DescriptorHeaps[Type[i]] = HeapPtrs[i];
			AnyChanged = true;
		}
	}


	if (AnyChanged)
	{
		BuildDescriptorHeaps();
	}
}

void FCommandContext::SetPipelineState(const FPSO& Pso)
{
	ID3D12PipelineState* PipelineS = Pso.GetPipelineStateObject();
	if (PipelineS == PipelineState)
	{
		return;
	}

	CommandList->SetPipelineState(PipelineS);
	PipelineState = PipelineS;
}

void FCommandContext::SetPrediction(ID3D12Resource* Buffer, uint64_t BufferOffset, D3D12_PREDICATION_OP Op)
{
	CommandList->SetPredication(Buffer, BufferOffset, Op);
}



///for graphic command context
void FGraphicsContext::ClearUAV(FColorBuffer& Target)
{
	FlushResourceBarriers();

	//after binding a uav we can get a gpu handle that is required to clear it as uav(ut essentially run a  shader to set all of values)
	D3D12_GPU_DESCRIPTOR_HANDLE GpuVisibleHandle = DynamicViewDescriptorHeap.UploadDirect(Target.GetUav());
	CD3DX12_RECT ClearRect(0, 0, Target.GetWidth(), Target.GetHeight());

	const float* ClearColor = Target.GetClearColor().GetPtr();
	CommandList->ClearUnorderedAccessViewFloat(GpuVisibleHandle, Target.GetUav(), Target.GetResource(), ClearColor, 1, &ClearRect);
}

void FGraphicsContext::ClearUAV(FGpuBuffer& Target)
{
	FlushResourceBarriers();

	D3D12_GPU_DESCRIPTOR_HANDLE GpuVisibleHandle = DynamicViewDescriptorHeap.UploadDirect(Target.GetUAV());
	const UINT ClearColor[4] = {};
	CommandList->ClearUnorderedAccessViewUint(GpuVisibleHandle, Target.GetUAV(), Target.GetResource(), ClearColor, 0, nullptr);
}

void FGraphicsContext::ClearColor(FColorBuffer& Target, D3D12_RECT* Rect /* = nullptr */)
{
	FlushResourceBarriers();
	CommandList->ClearRenderTargetView(Target.GetRtv(), Target.GetClearColor().GetPtr(), (Rect == nullptr) ? 0 : 1, Rect);
}

void FGraphicsContext::ClearColor(FColorBuffer& Target, float Colour[4], D3D12_RECT* Rect /* = nullptr */)
{
	FlushResourceBarriers();
	CommandList->ClearRenderTargetView(Target.GetRtv(), Colour, (Rect == nullptr) ? 0 : 1, Rect);
}

void FGraphicsContext::ClearDepth(FDepthBuffer& Target)
{
	FlushResourceBarriers();
	CommandList->ClearDepthStencilView(Target.GetDsv(), D3D12_CLEAR_FLAG_DEPTH, Target.GetClearDepth(),
		Target.GetClearStencil(), 0, nullptr);
}

void FGraphicsContext::ClearStencil(FDepthBuffer& Target)
{
	FlushResourceBarriers();
	CommandList->ClearDepthStencilView(Target.GetDsv(), D3D12_CLEAR_FLAG_STENCIL, Target.GetClearDepth(),
		Target.GetClearStencil(), 0, nullptr);
}

void FGraphicsContext::ClearDepthAndStencil(FDepthBuffer& Target)
{
	FlushResourceBarriers();
	CommandList->ClearDepthStencilView(Target.GetDsv(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, Target.GetClearDepth(),
		Target.GetClearStencil(), 0, nullptr);
}

void FGraphicsContext::SetStencilRef(UINT StencilRef)
{
	CommandList->OMSetStencilRef(StencilRef);
}

void FGraphicsContext::SetBlendFactor(FColor BlendFactor)
{
	CommandList->OMSetBlendFactor(BlendFactor.GetPtr());
}

void FGraphicsContext::SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY InTopology)
{
	CommandList->IASetPrimitiveTopology(InTopology);
}


void FGraphicsContext::BeignQuery(ID3D12QueryHeap* QueryHeap, D3D12_QUERY_TYPE InType, UINT HeapIndex)
{
	CommandList->BeginQuery(QueryHeap, InType, HeapIndex);
}

void FGraphicsContext::EndQuery(ID3D12QueryHeap* QueryHeap, D3D12_QUERY_TYPE InType, UINT HeapIndex)
{
	CommandList->EndQuery(QueryHeap, InType, HeapIndex);
}

void FGraphicsContext::ResolveQueryData(ID3D12QueryHeap* QueryHeap, D3D12_QUERY_TYPE InType, UINT StartIndex, UINT NumQueries,
	ID3D12Resource* DestinationBuffer, UINT64 DesctinationBufferOffset)
{
	CommandList->ResolveQueryData(QueryHeap, InType, StartIndex, NumQueries, DestinationBuffer, DesctinationBufferOffset);
}

void FGraphicsContext::SetRootSignature(const FRootSignature& RootSig)
{
	if (RootSig.GetSignature() == GraphicsRootSignature)
	{
		return;
	}

	CommandList->SetGraphicsRootSignature(GraphicsRootSignature = RootSig.GetSignature());

	DynamicViewDescriptorHeap.ParseGraphicsRootSignature(RootSig);
	DynamicSamplerDescriptorHeap.ParseGraphicsRootSignature(RootSig);
}

void FGraphicsContext::SetRenderTargets(UINT NumRtvs, const D3D12_CPU_DESCRIPTOR_HANDLE Rtvs[])
{
	CommandList->OMSetRenderTargets(NumRtvs, Rtvs, FALSE, nullptr);
}

void FGraphicsContext::SetRenderTargets(UINT NumRtvs, const D3D12_CPU_DESCRIPTOR_HANDLE Rtvs[], D3D12_CPU_DESCRIPTOR_HANDLE Dsv)
{
	CommandList->OMSetRenderTargets(NumRtvs, Rtvs, FALSE, &Dsv);
}

void FGraphicsContext::SetViewport(const D3D12_VIEWPORT& Vp)
{
	CommandList->RSSetViewports(1, &Vp);
}

void FGraphicsContext::SetViewport(FLOAT X, FLOAT Y, FLOAT W, FLOAT H, FLOAT MinDepth, FLOAT MaxDepth)
{
	D3D12_VIEWPORT Vp;
	Vp.TopLeftX = X;
	Vp.TopLeftY = Y;
	Vp.Width = W;
	Vp.Height = H;
	Vp.MinDepth = MinDepth;
	Vp.MaxDepth = MaxDepth;

	CommandList->RSSetViewports(1, &Vp);
}

void FGraphicsContext::SetScissor(const D3D12_RECT& Rect)
{
	ASSERT(Rect.left < Rect.right&& Rect.top < Rect.bottom);
	CommandList->RSSetScissorRects(1, &Rect);
}

void FGraphicsContext::SetViewportAndScissor(const D3D12_VIEWPORT& Vp, const D3D12_RECT& rect)
{
	SetViewport(Vp);
	SetScissor(rect);
}

void FGraphicsContext::SetScissor(UINT Left, UINT Top, UINT Right, UINT Bottom)
{
	SetScissor(CD3DX12_RECT(Left, Top, Right, Bottom));
}

void FGraphicsContext::SetViewportAndScissor(UINT X, UINT Y, UINT W, UINT H)
{
	SetViewport((float)X, (float)Y, (float)W, (float)H);
	SetScissor(X, Y, X + W, Y + H);
}


void FGraphicsContext::SetConstant(UINT RootEntry, UINT Offset, FDWParam Val)
{
	CommandList->SetGraphicsRoot32BitConstant(RootEntry, Val.Uint, Offset);
}

void FGraphicsContext::SetConstants(UINT RootIndex, FDWParam X, FDWParam Y, FDWParam Z, FDWParam W)
{
	CommandList->SetGraphicsRoot32BitConstant(RootIndex, X.Uint, 0);
	CommandList->SetGraphicsRoot32BitConstant(RootIndex, Y.Uint, 1);
	CommandList->SetGraphicsRoot32BitConstant(RootIndex, Z.Uint, 2);
	CommandList->SetGraphicsRoot32BitConstant(RootIndex, W.Uint, 3);
}

void FGraphicsContext::SetConstants(UINT RootIndex, FDWParam X, FDWParam Y, FDWParam Z)
{
	CommandList->SetGraphicsRoot32BitConstant(RootIndex, X.Uint, 0);
	CommandList->SetGraphicsRoot32BitConstant(RootIndex, Y.Uint, 1);
	CommandList->SetGraphicsRoot32BitConstant(RootIndex, Z.Uint, 2);
}

void FGraphicsContext::SetConstants(UINT RootIndex, FDWParam X)
{
	CommandList->SetGraphicsRoot32BitConstant(RootIndex, X.Uint, 0);
}

void FGraphicsContext::SetConstants(UINT RootIndex, FDWParam X, FDWParam Y)
{
	CommandList->SetGraphicsRoot32BitConstant(RootIndex, X.Uint, 0);
	CommandList->SetGraphicsRoot32BitConstant(RootIndex, Y.Uint, 1);
}

void FGraphicsContext::SetConstantArray(UINT RootIndex, UINT NumConstants, const void* ContantsData)
{
	CommandList->SetGraphicsRoot32BitConstants(RootIndex, NumConstants, ContantsData, 0);
}

void FGraphicsContext::SetConstantBuffer(UINT RootIndex, D3D12_GPU_VIRTUAL_ADDRESS Cbv)
{
	CommandList->SetGraphicsRootConstantBufferView(RootIndex, Cbv);
}

void FGraphicsContext::SetDynamicConstantBufferView(UINT RootIndex, size_t BufferSize, const void* BufferData)
{
	ASSERT(BufferData != nullptr && Math::IsAligned(BufferData, 16));
	FDynAlloc Cb = CpuLinearAllocator.Allocate(BufferSize);
	memcpy(Cb.DataPtr, BufferData, BufferSize);
	CommandList->SetGraphicsRootConstantBufferView(RootIndex, Cb.GpuAddress);

}

void FGraphicsContext::SetBufferSrv(UINT RootIndex, FGpuBuffer& InSrv, UINT64 Offset /* = 0 */)
{
	D3D12_RESOURCE_STATES ResourceState = InSrv.GetResourceState();
	ASSERT(ResourceState & D3D12_RESOURCE_STATE_UNORDERED_ACCESS != 0);
	CommandList->SetGraphicsRootShaderResourceView(RootIndex, InSrv.GetGpuVirtualAddress() + Offset);
}

void FGraphicsContext::SetBufferUav(UINT RootIndex, FGpuBuffer& Uav, UINT64 Offset /* = 0 */)
{
	D3D12_RESOURCE_STATES ResourceState = Uav.GetResourceState();
	ASSERT(ResourceState & D3D12_RESOURCE_STATE_UNORDERED_ACCESS != 0);
	CommandList->SetGraphicsRootUnorderedAccessView(RootIndex, Uav.GetGpuVirtualAddress() + Offset);
}

void FGraphicsContext::SetDescriptorTable(UINT RootIndex, D3D12_GPU_DESCRIPTOR_HANDLE FirstHandle)
{
	CommandList->SetGraphicsRootDescriptorTable(RootIndex, FirstHandle);
}

void FGraphicsContext::SetDynamicDescriptor(UINT RootIndex, UINT Offset, D3D12_CPU_DESCRIPTOR_HANDLE Handle)
{
	SetDynamicDescriptors(RootIndex, Offset, 1, &Handle);
}

void FGraphicsContext::SetDynamicDescriptors(UINT RootIndex, UINT Offset, UINT Count, const D3D12_CPU_DESCRIPTOR_HANDLE Handles[])
{
	DynamicViewDescriptorHeap.SetGraphicsDescriptorHandles(RootIndex, Offset, Count, Handles);
}

void FGraphicsContext::SetDynamicSampler(UINT RootIndex, UINT Offset, D3D12_CPU_DESCRIPTOR_HANDLE Handle)
{
	SetDynamicSamplers(RootIndex, Offset, 1, &Handle);
}

void FGraphicsContext::SetDynamicSamplers(UINT RootIndex, UINT Offset, UINT Count, const D3D12_CPU_DESCRIPTOR_HANDLE Handles[])
{
	DynamicSamplerDescriptorHeap.SetGraphicsDescriptorHandles(RootIndex, Offset, Count, Handles);
}

void FGraphicsContext::SetIndexBuffer(const D3D12_INDEX_BUFFER_VIEW& IbView)
{
	CommandList->IASetIndexBuffer(&IbView);
}

void FGraphicsContext::SetVertexBuffer(UINT Slot, const D3D12_VERTEX_BUFFER_VIEW& VbView)
{
	CommandList->IASetVertexBuffers(Slot, 1, &VbView);
}

void FGraphicsContext::SetVertexBuffers(UINT StartSlot, UINT Count, const D3D12_VERTEX_BUFFER_VIEW VbViews[])
{
	CommandList->IASetVertexBuffers(StartSlot, Count, VbViews);
}

void FGraphicsContext::SetDynamicIB(size_t IndexCount, const uint16_t* IBData)
{
	ASSERT(IBData != nullptr && Math::IsAligned(IBData, 16));
	
	size_t BufferSize = Math::AlignUp(IndexCount * sizeof(uint16_t), 16);
	FDynAlloc IB = CpuLinearAllocator.Allocate(BufferSize);
	SIMDMemCopy(IB.DataPtr, IBData, BufferSize >> 4);

	D3D12_INDEX_BUFFER_VIEW IBView;
	IBView.BufferLocation = IB.GpuAddress;
	IBView.SizeInBytes = (UINT)(IndexCount * sizeof(uint16_t));
	IBView.Format = DXGI_FORMAT_R16_UINT;

	CommandList->IASetIndexBuffer(&IBView);

}

void FGraphicsContext::SetDynamicVB(UINT Slot, size_t NumVertices, size_t VertexStride, const void* VBData)
{
	ASSERT(VBData != nullptr && Math::IsAligned(VBData, 16));

	size_t BufferSize = Math::AlignUp(NumVertices * VertexStride, 16);
	FDynAlloc VB = CpuLinearAllocator.Allocate(BufferSize);
	SIMDMemCopy(VB.DataPtr, VBData, BufferSize >> 4);

	D3D12_VERTEX_BUFFER_VIEW VBView;
	VBView.BufferLocation = VB.GpuAddress;
	VBView.SizeInBytes = (UINT)BufferSize;
	VBView.StrideInBytes = VertexStride;

	CommandList->IASetVertexBuffers(Slot, 1, &VBView);
}

void FGraphicsContext::SetDynamicSrv(UINT RootIndex, size_t BufferSize, const void* BufferData)
{
	ASSERT(BufferData != nullptr && Math::IsAligned(BufferData, 16));

	FDynAlloc Cb = CpuLinearAllocator.Allocate(BufferSize);
	SIMDMemCopy(Cb.DataPtr, BufferData, Math::AlignUp(BufferSize, 16) >> 4);
	CommandList->SetGraphicsRootShaderResourceView(RootIndex, Cb.GpuAddress);
}

//draw interface
void FGraphicsContext::Draw(UINT VertexCount, UINT VertexStartOffset /* = 0 */)
{
	DrawInstanced(VertexCount, 1, VertexStartOffset, 0);
}

void FGraphicsContext::DrawIndexed(UINT IndexCount, UINT StartIndexLocation /* = 0 */, UINT BaseVertexLocation /* = 0 */)
{
	DrawIndexedInstanced(IndexCount, 1, StartIndexLocation, BaseVertexLocation, 0);
}

void FGraphicsContext::DrawInstanced(UINT VertexCountPerInstance, UINT InstanceCount,
	UINT StartVertexLocation /* = 0 */, UINT StartIndexLocation /* = 0 */)
{
	FlushResourceBarriers();

	DynamicViewDescriptorHeap.CommitGraphicsRootDescriptorTables(CommandList);
	DynamicSamplerDescriptorHeap.CommitGraphicsRootDescriptorTables(CommandList);
	CommandList->DrawInstanced(VertexCountPerInstance, InstanceCount, StartVertexLocation, StartIndexLocation);
}

void FGraphicsContext::DrawIndexedInstanced(UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation)
{
	FlushResourceBarriers();

	DynamicViewDescriptorHeap.CommitGraphicsRootDescriptorTables(CommandList);
	DynamicSamplerDescriptorHeap.CommitGraphicsRootDescriptorTables(CommandList);
	CommandList->DrawIndexedInstanced(IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
}

void FGraphicsContext::DrawIndirect(FGpuBuffer& ArgumentBuffer, uint64_t ArgumentBufferOffset /* = 0 */)
{
	ExcuteIndirect(Graphics::DrawIndirectCommandSignature, ArgumentBuffer, ArgumentBufferOffset);
}

void FGraphicsContext::ExcuteIndirect(FCommandSignature& CommandSig, FGpuBuffer& ArgumentBuffer, 
	uint64_t ArgumentStartOffset /* = 0 */, uint32_t MaxComamnds /* = 1 */, FGpuBuffer* CommandCounterBuffer /* = nullptr */, uint64_t CounterOffset /* = 0 */)
{
	FlushResourceBarriers();

	DynamicViewDescriptorHeap.CommitGraphicsRootDescriptorTables(CommandList);
	DynamicSamplerDescriptorHeap.CommitGraphicsRootDescriptorTables(CommandList);
	CommandList->ExecuteIndirect(CommandSig.GetSignature(), MaxComamnds, ArgumentBuffer.GetResource(),
		ArgumentStartOffset,
		CommandCounterBuffer == nullptr ? nullptr : CommandCounterBuffer->GetResource(), CounterOffset);
}