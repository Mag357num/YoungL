#pragma once
#include "../../pch.h"
#include "DescriptorHeap.h"
#include <vector>
#include <queue>

class FCommandContext;
class FRootSignature;

class FDynamicDescriptorHeap
{
public:
	FDynamicDescriptorHeap(FCommandContext& CmdContext, D3D12_DESCRIPTOR_HEAP_TYPE HeapType);
	~FDynamicDescriptorHeap();

	static void DestroyAll()
	{
		DescriptorHeapPool[0].clear();
		DescriptorHeapPool[1].clear();
	}

	void CleanUpUsedHeaps(uint64_t fenceValue);

	// copy multiple handles into the cache area reserved for the specified root parameter;
	void SetGraphicsDescriptorHandles(UINT RootIndex, UINT Offset, UINT NumHandles, const D3D12_CPU_DESCRIPTOR_HANDLE Handles[])
	{
		GraphicsHandleCache.StageDescriptorHandles(RootIndex, Offset, NumHandles, Handles);
	}

	void SetComputeDescriptorHandles(UINT RootIndex, UINT Offset, UINT NumHandles, const D3D12_CPU_DESCRIPTOR_HANDLE Handles[])
	{
		ComputeHandleCache.StageDescriptorHandles(RootIndex, Offset, NumHandles, Handles);
	}

	//bypass the cache and upload directly to the shader visible heap
	D3D12_GPU_DESCRIPTOR_HANDLE UploadDirect(D3D12_CPU_DESCRIPTOR_HANDLE Handle);

	//deduce cache layout needed to support the descriptor tables needed by the root signatuer.
	void ParseGraphicsRootSignature(const FRootSignature& RootSig)
	{
		GraphicsHandleCache.ParseRootSignature(DescriptorType, RootSig);
	}

	void ParseComputeRootSignature(const FRootSignature& RootSig)
	{
		ComputeHandleCache.ParseRootSignature(DescriptorType, RootSig);
	}

	//upload any new descriptors in the cahe to the shader visible heap
	inline void CommitGraphicsRootDescriptorTables(ID3D12GraphicsCommandList* InCmdList)
	{
		if (GraphicsHandleCache.StaleRootParamsBitMap !=0)
		{
			CopyAndBindStagedtables(GraphicsHandleCache, InCmdList, &ID3D12GraphicsCommandList::SetGraphicsRootDescriptorTable);
		}
	}

	inline void CommitComuteRootDescriptortables(ID3D12GraphicsCommandList* InCmdList)
	{
		if (ComputeHandleCache.StaleRootParamsBitMap != 0)
		{
			CopyAndBindStagedtables(ComputeHandleCache, InCmdList, &ID3D12GraphicsCommandList::SetGraphicsRootDescriptorTable);
		}
	}


private:
	static const uint32_t NumDescriptorsPerHeap = 1024;
	static std::mutex HeapMutex;
	static std::vector<Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>> DescriptorHeapPool[2];
	static std::queue<std::pair<uint64_t, ID3D12DescriptorHeap*>> RetiredDescriptorHeaps[2];
	static std::queue<ID3D12DescriptorHeap*> AvailableDescriptorHeaps[2];

	static ID3D12DescriptorHeap* RequestDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE NewType);
	static void DiscardDiscriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE HeapType, uint64_t FenceValue,
		const std::vector<ID3D12DescriptorHeap*>& UsedHeaps);

	FCommandContext& OwingCommandContext;
	ID3D12DescriptorHeap* CurrentHeapPtr;
	const D3D12_DESCRIPTOR_HEAP_TYPE DescriptorType;
	uint32_t DescriptorSize;
	uint32_t CurrentOffset;
	FDescriptorHandle FirstDescriptor;
	std::vector<ID3D12DescriptorHeap*> RetiredHeaps;

	struct FDescriptorTableCache
	{
		FDescriptorTableCache() :AssignedHandlesBitMap(0) {}

		uint32_t AssignedHandlesBitMap;
		D3D12_CPU_DESCRIPTOR_HANDLE* TableStart;
		uint32_t TableSize;
	};

	struct FDescriptorHandleCache
	{
		FDescriptorHandleCache()
		{
			ClearCache();
		}

		void ClearCache()
		{
			RootDescriptorTablesBitmap = 0;
			MaxCachedDescriptors = 0;
		}

		uint32_t RootDescriptorTablesBitmap;
		uint32_t StaleRootParamsBitMap;
		uint32_t MaxCachedDescriptors;

		static const uint32_t MaxNumDescriptors = 256;
		static const uint32_t MaxNumDescriptorTables = 16;

		uint32_t ComputeStagedSize();
		void CopyAndBindStaleTables(D3D12_DESCRIPTOR_HEAP_TYPE InType, uint32_t DescriptorSize, FDescriptorHandle DeshandleStart,
			ID3D12GraphicsCommandList* CmdList, void(STDMETHODCALLTYPE ID3D12GraphicsCommandList::*SetFunc)(UINT, D3D12_GPU_DESCRIPTOR_HANDLE));

		FDescriptorTableCache RootDescriptorTable[MaxNumDescriptorTables];
		D3D12_CPU_DESCRIPTOR_HANDLE HandleCache[MaxNumDescriptors];

		void UnbindAllValid();
		void StageDescriptorHandles(UINT RootIndex, UINT Offset, UINT NumHandles, const D3D12_CPU_DESCRIPTOR_HANDLE Handles[]);
		void ParseRootSignature(D3D12_DESCRIPTOR_HEAP_TYPE InType, const FRootSignature& RootSig);

	};

	FDescriptorHandleCache GraphicsHandleCache;
	FDescriptorHandleCache ComputeHandleCache;

	bool HasSpace(uint32_t Count)
	{
		return CurrentHeapPtr != nullptr && CurrentOffset + Count <= NumDescriptorsPerHeap;
	}

	void RetiredCurrentHeap(void);
	void RetiredUsedHeaps(uint64_t FenceValue);
	ID3D12DescriptorHeap* GetHeapPointer();

	FDescriptorHandle Allocate(UINT Count)
	{
		FDescriptorHandle Ret = FirstDescriptor + CurrentOffset * DescriptorSize;
		CurrentOffset += Count;
		return Ret;
	}

	void CopyAndBindStagedtables(FDescriptorHandleCache& HandleCache, ID3D12GraphicsCommandList* CmdList,
		void(STDMETHODCALLTYPE ID3D12GraphicsCommandList::*SetFunc)(UINT, D3D12_GPU_DESCRIPTOR_HANDLE));

	void UnBindAllValid(void);

};