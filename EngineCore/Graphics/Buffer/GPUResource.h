#pragma once

class GPUResource
{
public:
	GPUResource():
		Y_GpuVirtualAddress(D3D12_GPU_VIRTUAL_ADDRESS_NULL),
		Y_UserAllocatedMemory(nullptr),
		Y_UsageState(D3D12_RESOURCE_STATE_COMMON),
		Y_TransitionState((D3D12_RESOURCE_STATES) - 1)

	{
	}

	GPUResource(ID3D12Resource* pResource, D3D12_RESOURCE_STATES CurrentState) :
		Y_GpuVirtualAddress(D3D12_GPU_VIRTUAL_ADDRESS_NULL),
		Y_UserAllocatedMemory(nullptr),
		Y_Resource(pResource),
		Y_UsageState(CurrentState),
		Y_TransitionState((D3D12_RESOURCE_STATES)-1)
	{
	}

	virtual void Destroy()
	{
		Y_Resource = nullptr;
		Y_GpuVirtualAddress = D3D12_GPU_VIRTUAL_ADDRESS_NULL;
		if (Y_UserAllocatedMemory != nullptr)
		{
			VirtualFree(Y_UserAllocatedMemory, 0, MEM_RELEASE);
			Y_UserAllocatedMemory = nullptr;
		}
	}

	ID3D12Resource* operator->() { return Y_Resource.Get(); }
	const ID3D12Resource* operator->() const { return Y_Resource.Get(); }

	ID3D12Resource* GetResource() { return Y_Resource.Get(); }
	const ID3D12Resource* GetResource() const { return Y_Resource.Get(); }

	D3D12_RESOURCE_STATES GetResourceState() { return Y_UsageState; }
	void SetResourceState(D3D12_RESOURCE_STATES InState) { Y_UsageState = InState; }
	D3D12_RESOURCE_STATES GetResourceTransitionState() { return Y_TransitionState; }
	void SetResourceTransitionState(D3D12_RESOURCE_STATES InState) { Y_TransitionState = InState; }

	D3D12_GPU_VIRTUAL_ADDRESS GetGpuVirtualAddress() const { return Y_GpuVirtualAddress; }

protected:
	Microsoft::WRL::ComPtr<ID3D12Resource> Y_Resource;
	D3D12_RESOURCE_STATES Y_UsageState;
	D3D12_RESOURCE_STATES Y_TransitionState;
	D3D12_GPU_VIRTUAL_ADDRESS Y_GpuVirtualAddress;

	//when using virtualalloc to allocate memory directly. record the allocation here, so it can be freed
	void* Y_UserAllocatedMemory;

};