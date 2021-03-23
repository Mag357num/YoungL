#pragma once

#include "../../pch.h"

class FGPUResource
{
public:
	FGPUResource():
		GpuVirtualAddress(D3D12_GPU_VIRTUAL_ADDRESS_NULL),
		UserAllocatedMemory(nullptr),
		UsageState(D3D12_RESOURCE_STATE_COMMON),
		TransitionState((D3D12_RESOURCE_STATES) - 1)

	{
	}

	FGPUResource(ID3D12Resource* pResource, D3D12_RESOURCE_STATES CurrentState) :
		GpuVirtualAddress(D3D12_GPU_VIRTUAL_ADDRESS_NULL),
		UserAllocatedMemory(nullptr),
		Resource(pResource),
		UsageState(CurrentState),
		TransitionState((D3D12_RESOURCE_STATES)-1)
	{
	}

	virtual void Destroy()
	{
		Resource = nullptr;
		GpuVirtualAddress = D3D12_GPU_VIRTUAL_ADDRESS_NULL;
		if (UserAllocatedMemory != nullptr)
		{
			VirtualFree(UserAllocatedMemory, 0, MEM_RELEASE);
			UserAllocatedMemory = nullptr;
		}
	}

	ID3D12Resource* operator->() { return Resource.Get(); }
	const ID3D12Resource* operator->() const { return Resource.Get(); }

	ID3D12Resource* GetResource() { return Resource.Get(); }
	const ID3D12Resource* GetResource() const { return Resource.Get(); }

	D3D12_RESOURCE_STATES GetResourceState() { return UsageState; }
	void SetResourceState(D3D12_RESOURCE_STATES InState) { UsageState = InState; }
	D3D12_RESOURCE_STATES GetResourceTransitionState() { return TransitionState; }
	void SetResourceTransitionState(D3D12_RESOURCE_STATES InState) { TransitionState = InState; }

	D3D12_GPU_VIRTUAL_ADDRESS GetGpuVirtualAddress() const { return GpuVirtualAddress; }

protected:

	Microsoft::WRL::ComPtr<ID3D12Resource> Resource;
	D3D12_RESOURCE_STATES UsageState;
	D3D12_RESOURCE_STATES TransitionState;
	D3D12_GPU_VIRTUAL_ADDRESS GpuVirtualAddress;

	//when using virtualalloc to allocate memory directly. record the allocation here, so it can be freed
	void* UserAllocatedMemory;

};