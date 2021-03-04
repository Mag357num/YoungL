#pragma once

class GPUResource
{
public:
	GPUResource():
		m_GpuVirtualAddress(D3D12_GPU_VIRTUAL_ADDRESS_NULL),
		m_UserAllocatedMemory(nullptr),
		m_UsageState(D3D12_RESOURCE_STATE_COMMON),
		m_TransitionState((D3D12_RESOURCE_STATES) - 1)

	{
	}

	GPUResource(ID3D12Resource* pResource, D3D12_RESOURCE_STATES CurrentState) :
		m_GpuVirtualAddress(D3D12_GPU_VIRTUAL_ADDRESS_NULL),
		m_UserAllocatedMemory(nullptr),
		m_Resource(pResource),
		m_UsageState(CurrentState),
		m_TransitionState((D3D12_RESOURCE_STATES)-1)
	{
	}

	virtual void Destroy()
	{
		m_Resource = nullptr;
		m_GpuVirtualAddress = D3D12_GPU_VIRTUAL_ADDRESS_NULL;
		if (m_UserAllocatedMemory != nullptr)
		{
			VirtualFree(m_UserAllocatedMemory, 0, MEM_RELEASE);
			m_UserAllocatedMemory = nullptr;
		}
	}

	ID3D12Resource* operator->() { return m_Resource.Get(); }
	const ID3D12Resource* operator->() const { return m_Resource.Get(); }

	ID3D12Resource* GetResource() { return m_Resource.Get(); }
	const ID3D12Resource* GetResource() const { return m_Resource.Get(); }

	D3D12_GPU_VIRTUAL_ADDRESS GetGpuVirtualAddress() const { return m_GpuVirtualAddress; }

protected:
	Microsoft::WRL::ComPtr<ID3D12Resource> m_Resource;
	D3D12_RESOURCE_STATES m_UsageState;
	D3D12_RESOURCE_STATES m_TransitionState;
	D3D12_GPU_VIRTUAL_ADDRESS m_GpuVirtualAddress;

	//when using virtualalloc to allocate memory directly. record the allocation here, so it can be freed
	void* m_UserAllocatedMemory;

};