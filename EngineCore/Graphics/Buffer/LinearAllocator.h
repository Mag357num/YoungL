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

class FLinearAllocator : public FGPUResource
{
public:
	FLinearAllocator(ID3D12Resource* PResource, D3D12_RESOURCE_STATES Usage) 
		: FGPUResource()
	{
		Resource.Attach(PResource);
		UsageState = Usage;

		GpuVirtualAddress = Resource->GetGPUVirtualAddress();
		Resource->Map(0, nullptr, &CpuVirtualAddress);
	}

	~FLinearAllocator()
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
