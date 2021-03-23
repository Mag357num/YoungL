// Description:  This is a dynamic graphics memory allocator for DX12.  It's designed to work in concert
// with the CommandContext class and to do so in a thread-safe manner.  There may be many command contexts,
// each with its own linear allocators.  They act as windows into a global memory pool by reserving a
// context-local memory page.  Requesting a new page is done in a thread-safe manner by guarding accesses
// with a mutex lock.
//
// When a command context is finished, it will receive a fence ID that indicates when it's safe to reclaim
// used resources.  The CleanupUsedPages() method must be invoked at this time so that the used pages can be
// scheduled for reuse after the fence has cleared.

#pragma once

#include "../../pch.h"
#include "GPUResource.h"

#include <vector>
#include <queue>

#define DEFAULT_ALIGN 256

struct FDynAlloc
{
	FDynAlloc(FGPUResource& BaseResource, size_t InOffset, size_t InSize)
		:Buffer(BaseResource),
		Offset(InOffset),
		Size(InSize)
	{}

	FGPUResource& Buffer;
	size_t Offset;
	size_t Size;
	void* DataPtr;
	D3D12_GPU_VIRTUAL_ADDRESS GpuAddress;
};

class FLinearAllocationPage : public FGPUResource
{
public:
	FLinearAllocationPage(ID3D12Resource* PResource, D3D12_RESOURCE_STATES Usage)
		: FGPUResource()
	{
		Resource.Attach(PResource);
		UsageState = Usage;

		GpuVirtualAddress = Resource->GetGPUVirtualAddress();
		Resource->Map(0, nullptr, &CpuVirtualAddress);
	}

	~FLinearAllocationPage()
	{
		Unmap();
	}

	void Map()
	{
		if (CpuVirtualAddress == nullptr)
		{
			Resource->Map(0, nullptr, &CpuVirtualAddress);
		}
	}

	void Unmap()
	{
		if (CpuVirtualAddress!=nullptr)
		{
			Resource->Unmap(0, nullptr);
			CpuVirtualAddress = nullptr;
		}
	}

	void* CpuVirtualAddress;
	D3D12_GPU_VIRTUAL_ADDRESS GpuVirtualAddress;

};

enum ELinearAllocatorType
{
	EInvalidAllocator = -1,
	EGpuExlusive = 0,
	ECpuWriteable = 1,
	
	ENumAllocatorTypes
};

enum 
{
	EGpuAllocatorPageSize = 0x10000,//64K
	ECpuAllocaterPageSize = 0x200000 //2M
};
class FLinearAllocatorPagemanager
{
public:
	FLinearAllocatorPagemanager();

	FLinearAllocationPage* RequestPage(void);
	FLinearAllocationPage* CreateNewPage(size_t PageSize= 0);

	void DiscardPages(uint64_t FenceID, const std::vector<FLinearAllocationPage*>& Pages);
	void FreeLargePools(uint64_t FenceID, const std::vector<FLinearAllocationPage*>& Pages);

	void Destroy(void){
		PagePool.clear();
	}
private:
	static ELinearAllocatorType AutoType;

	ELinearAllocatorType AllocationType;
	std::vector<std::unique_ptr<FLinearAllocationPage>> PagePool;
	std::queue<std::pair<uint64_t, FLinearAllocationPage*>> RetiredPages;
	std::queue<std::pair<uint64_t, FLinearAllocationPage*>> DeletionQueue;
	std::queue<FLinearAllocationPage*> AvailablePages;
	std::mutex AllocateMutex;

};

class FLinearAllocator
{
public:
	FLinearAllocator(ELinearAllocatorType Type)
		:AllocationType(Type),
		PageSize(0),
		CurOffset(~(size_t)0),
		CurPage(nullptr)
	{
		ASSERT(Type > EInvalidAllocator && Type < ENumAllocatorTypes);
		PageSize = (Type == EGpuExlusive ? EGpuAllocatorPageSize : ECpuAllocaterPageSize);
	}

	FDynAlloc Allocate(size_t SizeInBytes, size_t Alignment = DEFAULT_ALIGN);

	void ClearUpUsedPages(uint64_t FenceID);

	static void DestroALl(void)
	{
		PageManager[0].Destroy();
		PageManager[1].Destroy();
	}
	
private:
	FDynAlloc AllocateLargePage(size_t SizeInBytes);

	static FLinearAllocatorPagemanager PageManager[2];

	ELinearAllocatorType AllocationType;
	size_t PageSize;
	size_t CurOffset;
	FLinearAllocationPage* CurPage;
	std::vector<FLinearAllocationPage*> RetiredPages;
	std::vector<FLinearAllocationPage*> LargePageList;
};
