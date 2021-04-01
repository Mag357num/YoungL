#include "pch.h"
#include "DynamicDescriptorHeap.h"
#include "../GraphicsCore.h"

using namespace Graphics;

FDynamicDescriptorHeap::FDynamicDescriptorHeap(FCommandContext& CmdContext, D3D12_DESCRIPTOR_HEAP_TYPE HeapType)
	:OwingCommandContext(CmdContext),
	DescriptorType(HeapType)
{
	CurrentHeapPtr = nullptr;
	CurrentOffset = 0;
	DescriptorSize = Graphics::g_Device->GetDescriptorHandleIncrementSize(HeapType);
}

FDynamicDescriptorHeap::~FDynamicDescriptorHeap()
{

}

uint32_t FDynamicDescriptorHeap::FDescriptorHandleCache::ComputeStagedSize()
{
	uint32_t NeedSpace = 0;
	uint32_t RootIndex = 0;
	uint32_t StaleParams = StaleRootParamsBitMap;
	while (_BitScanForward((unsigned long*)&RootIndex, StaleParams))
	{
		StaleParams ^= (1 << RootIndex);

		uint32_t MaxSetHandle;
		ASSERT(TRUE == _BitScanReverse((unsigned long*)&MaxSetHandle, RootDescriptorTable[RootIndex].AssignedHandlesBitMap),
			"Root entry marked as stale but has no stale descriptors");
		NeedSpace += MaxSetHandle + 1;
	}

	return NeedSpace;
}

void FDynamicDescriptorHeap::FDescriptorHandleCache::CopyAndBindStaleTables(D3D12_DESCRIPTOR_HEAP_TYPE InType, uint32_t DescriptorSize, 
	FDescriptorHandle DeshandleStart, 
	ID3D12GraphicsCommandList* CmdList, 
	void(STDMETHODCALLTYPE ID3D12GraphicsCommandList::* SetFunc)(UINT, D3D12_GPU_DESCRIPTOR_HANDLE))
{
	uint32_t StaleParamCount = 0;
	uint32_t TableSize[FDescriptorHandleCache::MaxNumDescriptorTables];
	uint32_t RootIndices[FDescriptorHandleCache::MaxNumDescriptorTables];
	uint32_t NeedSpace = 0;
	uint32_t RootIndex;

	uint32_t StaleParams = StaleRootParamsBitMap;
	while (_BitScanForward((unsigned long*)&RootIndex, StaleParams))
	{
		RootIndices[StaleParamCount] = RootIndex;
		StaleParams ^= (1 << RootIndex);

		uint32_t MaxSethandle = 0;
		ASSERT(TRUE == _BitScanReverse((unsigned long*)&MaxSethandle, RootDescriptorTable[RootIndex].AssignedHandlesBitMap),
			"Root entry marked as stale but has no stale descriptors");

		NeedSpace += MaxSethandle + 1;
		TableSize[StaleParamCount] = MaxSethandle + 1;

		++StaleParamCount;
	}

	ASSERT(StaleParamCount <= FDescriptorHandleCache::MaxNumDescriptorTables, "We're only equipped to handle so many descriptor tables");

	StaleRootParamsBitMap = 0;

	static const uint32_t MaxNumDescriptorsPerCopy = 16;
	UINT NumDestdescriptorRanges = 0;
	D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptorRangeStarts[MaxNumDescriptorsPerCopy];
	UINT DestDescriptorRangeSize[MaxNumDescriptorsPerCopy];

	UINT NumSrcDescriptorRanges = 0;
	D3D12_CPU_DESCRIPTOR_HANDLE SrcDescriptorRangeStarts[MaxNumDescriptorsPerCopy];
	UINT SrcDescriptorRangeSize[MaxNumDescriptorsPerCopy];

	for (uint32_t i = 0; i < StaleParamCount; ++i)
	{
		RootIndex = RootIndices[i];
		(CmdList->*SetFunc)(RootIndex, DeshandleStart);

		FDescriptorTableCache& RootDescTable = RootDescriptorTable[RootIndex];

		D3D12_CPU_DESCRIPTOR_HANDLE* SrcHandle = RootDescTable.TableStart;
		uint64_t Sethandles = (uint64_t)RootDescTable.AssignedHandlesBitMap;
		D3D12_CPU_DESCRIPTOR_HANDLE CurDest = DeshandleStart;
		DeshandleStart += TableSize[i] * DescriptorSize;

		unsigned long SkipCount;
		while (_BitScanForward(&SkipCount, Sethandles))
		{
			Sethandles >>= SkipCount;
			SrcHandle += SkipCount;
			CurDest.ptr += SkipCount * DescriptorSize;

			unsigned long DescriptorCount;
			_BitScanForward(&DescriptorCount, ~Sethandles);
			Sethandles >>= DescriptorCount;

			//if we run out of temp room, copy thwt we've got so far
			if (NumSrcDescriptorRanges + DescriptorCount > MaxNumDescriptorsPerCopy)
			{
				g_Device->CopyDescriptors(NumDestdescriptorRanges, DestDescriptorRangeStarts, DestDescriptorRangeSize,
					NumSrcDescriptorRanges, SrcDescriptorRangeStarts, SrcDescriptorRangeSize,
					InType);

				NumSrcDescriptorRanges = 0;
				NumDestdescriptorRanges = 0;
			}

			//set up destination rage
			DestDescriptorRangeStarts[NumDestdescriptorRanges] = CurDest;
			DestDescriptorRangeSize[NumDestdescriptorRanges] = DescriptorCount;
			++NumDestdescriptorRanges;


			//set up source ranges
			for (uint32_t j =0; j< DescriptorCount; ++j)
			{
				SrcDescriptorRangeStarts[NumSrcDescriptorRanges] = SrcHandle[i];
				SrcDescriptorRangeSize[NumSrcDescriptorRanges] = 1;
				NumSrcDescriptorRanges++;
			}

			//move the destination pointer forward by the naum of descriptors we will coyp
			SrcHandle += DescriptorCount;
			CurDest.ptr += DescriptorCount * DescriptorSize;
		}
	}

	g_Device->CopyDescriptors(NumDestdescriptorRanges, DestDescriptorRangeStarts, DestDescriptorRangeSize,
		NumSrcDescriptorRanges, SrcDescriptorRangeStarts, SrcDescriptorRangeSize,
		InType);

}

void FDynamicDescriptorHeap::FDescriptorHandleCache::UnbindAllValid()
{
	StaleRootParamsBitMap = 0;
	unsigned long TableParams = RootDescriptorTablesBitmap;
	unsigned long RootIndex;
	while (_BitScanForward(&RootIndex, TableParams))
	{
		TableParams ^= (1 << RootIndex);
		if (RootDescriptorTable[RootIndex].AssignedHandlesBitMap != 0)
		{
			StaleRootParamsBitMap |= (1 << RootIndex);
		}
	}
}

void FDynamicDescriptorHeap::FDescriptorHandleCache::StageDescriptorHandles(UINT RootIndex, UINT Offset, UINT NumHandles, const D3D12_CPU_DESCRIPTOR_HANDLE Handles[])
{
	ASSERT(((1 << RootIndex) & RootDescriptorTablesBitmap) != 0, "Root parameter is not a cbv_src_uav descriptor table");
	ASSERT(Offset + NumHandles <= RootDescriptorTable[RootIndex].TableSize);

	FDescriptorTableCache& TableCahe = RootDescriptorTable[RootIndex];
	D3D12_CPU_DESCRIPTOR_HANDLE* CopyDest = TableCahe.TableStart + Offset;
	for (UINT i = 0; i < NumHandles; i++)
	{
		CopyDest[i] = Handles[i];
	}

	TableCahe.AssignedHandlesBitMap |= ((1 << NumHandles) - 1) << Offset;
	StaleRootParamsBitMap |= (1 << RootIndex);
}

void FDynamicDescriptorHeap::FDescriptorHandleCache::ParseRootSignature(D3D12_DESCRIPTOR_HEAP_TYPE InType, const FRootSignature& RootSig)
{
	UINT CurrentOffset = 0;

	ASSERT(RootSig.NumParameter <= 16, "Maybe we need to support something greater");

	StaleRootParamsBitMap = 0;
	RootDescriptorTablesBitmap = (InType == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER) ? RootSig.SamplerTableBitMap : RootSig.DescriptorTableBitMap;

	unsigned long TableParams = RootDescriptorTablesBitmap;
	unsigned long RootIndex;
	while (_BitScanForward(&RootIndex, TableParams))
	{
		TableParams ^= (1 << RootIndex);

		UINT TableSize = RootSig.DescriptorTableSize[RootIndex];
		ASSERT(TableSize > 0);

		FDescriptorTableCache& RootTable = RootDescriptorTable[RootIndex];
		RootTable.AssignedHandlesBitMap = 0;
		RootTable.TableStart = HandleCache + CurrentOffset;
		RootTable.TableSize = TableSize;

		CurrentOffset += TableSize;
	}

	MaxCachedDescriptors = CurrentOffset;
	ASSERT(MaxCachedDescriptors < MaxNumDescriptors, "Exceeded user- supplied maximum cache size");
}

std::mutex FDynamicDescriptorHeap::HeapMutex;
std::vector<Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>> FDynamicDescriptorHeap::DescriptorHeapPool[2];
std::queue<std::pair<uint64_t, ID3D12DescriptorHeap*>> FDynamicDescriptorHeap::RetiredDescriptorHeaps[2];
std::queue<ID3D12DescriptorHeap*> FDynamicDescriptorHeap::AvailableDescriptorHeaps[2];

ID3D12DescriptorHeap* FDynamicDescriptorHeap::RequestDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE NewType)
{
	std::lock_guard<std::mutex> LocakGuard(HeapMutex);

	uint32_t Idx = NewType == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER ? 1 : 0;
	while (!RetiredDescriptorHeaps[Idx].empty() && g_CommandManager.IsFenceComplete(RetiredDescriptorHeaps[Idx].front().first))
	{
		AvailableDescriptorHeaps[Idx].push(RetiredDescriptorHeaps[Idx].front().second);
		RetiredDescriptorHeaps[Idx].pop();
	}

	if (!AvailableDescriptorHeaps[Idx].empty())
	{
		ID3D12DescriptorHeap* HeapPtr = AvailableDescriptorHeaps[Idx].front();
		AvailableDescriptorHeaps[Idx].pop();
		
		return HeapPtr;
	}
	else
	{
		D3D12_DESCRIPTOR_HEAP_DESC HeapDesc = {};
		HeapDesc.Type = NewType;
		HeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		HeapDesc.NodeMask = 1;
		HeapDesc.NumDescriptors = NumDescriptorsPerHeap;

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> HeapPtr;
		ASSERT_SUCCEEDED(g_Device->CreateDescriptorHeap(&HeapDesc, MY_IID_PPV_ARGS(&HeapPtr)));
		DescriptorHeapPool[Idx].emplace_back(HeapPtr);

		return HeapPtr.Get();

	}
}

void FDynamicDescriptorHeap::DiscardDiscriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE HeapType, uint64_t FenceValue, const std::vector<ID3D12DescriptorHeap*>& UsedHeaps)
{
	uint32_t Idx = HeapType == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER ? 1 : 0;
	
	std::lock_guard<std::mutex> LockGurad(HeapMutex);
	for (auto Iter = UsedHeaps.begin(); Iter != UsedHeaps.end() ; ++Iter)
	{
		RetiredDescriptorHeaps[Idx].push(std::make_pair(FenceValue, *Iter));
	}
}

void FDynamicDescriptorHeap::RetiredCurrentHeap()
{
	if (CurrentOffset ==0)
	{
		ASSERT(CurrentHeapPtr == nullptr);
		return;
	}

	ASSERT(CurrentHeapPtr != nullptr);
	RetiredHeaps.push_back(CurrentHeapPtr);
	CurrentHeapPtr = nullptr;
	CurrentOffset = 0;
}

void FDynamicDescriptorHeap::RetiredUsedHeaps(uint64_t FenceValue)
{
	DiscardDiscriptorHeap(DescriptorType, FenceValue, RetiredHeaps);
	RetiredHeaps.clear();
}

ID3D12DescriptorHeap* FDynamicDescriptorHeap::GetHeapPointer()
{
	if (CurrentHeapPtr == nullptr)
	{
		ASSERT(CurrentOffset == 0);
		CurrentHeapPtr = RequestDescriptorHeap(DescriptorType);
		FirstDescriptor = FDescriptorHandle(
		CurrentHeapPtr->GetCPUDescriptorHandleForHeapStart(),
			CurrentHeapPtr->GetGPUDescriptorHandleForHeapStart()
		);
	}

	return CurrentHeapPtr;
}

void FDynamicDescriptorHeap::CopyAndBindStagedtables(FDescriptorHandleCache& HandleCache, ID3D12GraphicsCommandList* CmdList, 
	void(STDMETHODCALLTYPE ID3D12GraphicsCommandList::* SetFunc)(UINT, D3D12_GPU_DESCRIPTOR_HANDLE))
{
	uint32_t NeededSize = HandleCache.ComputeStagedSize();
	if (!HasSpace(NeededSize))
	{
		RetiredCurrentHeap();
		UnBindAllValid();
		NeededSize = HandleCache.ComputeStagedSize();
	}

	// this can be triggerd the creation of new heap
	OwingCommandContext.SetDescriptorHeap(DescriptorType, GetHeapPointer());
	HandleCache.CopyAndBindStaleTables(DescriptorType, DescriptorSize, Allocate(NeededSize), CmdList, SetFunc);
}

void FDynamicDescriptorHeap::UnBindAllValid()
{
	GraphicsHandleCache.UnbindAllValid();
	ComputeHandleCache.UnbindAllValid();
}

void FDynamicDescriptorHeap::CleanUpUsedHeaps(uint64_t fenceValue)
{
	RetiredCurrentHeap();
	RetiredUsedHeaps(fenceValue);
	GraphicsHandleCache.ClearCache();
	ComputeHandleCache.ClearCache();
}

D3D12_GPU_DESCRIPTOR_HANDLE FDynamicDescriptorHeap::UploadDirect(D3D12_CPU_DESCRIPTOR_HANDLE Handle)
{
	if (!HasSpace(1))
	{
		RetiredCurrentHeap();
		UnBindAllValid();
	}

	OwingCommandContext.SetDescriptorHeap(DescriptorType, GetHeapPointer());

	FDescriptorHandle DestHandle = FirstDescriptor + CurrentOffset * DescriptorSize;
	CurrentOffset += 1;

	g_Device->CopyDescriptorsSimple(1, DestHandle, Handle, DescriptorType);

	return DestHandle;
}