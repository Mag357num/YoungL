#pragma once
#include "../RHIContext.h"
#include "RHIUploadBuffer_D3D12.h"

template<typename T>
class FRHIConstantBuffer_D3D12 : public IRHIConstantBuffer<T>
{
public:
	FRHIConstantBuffer_D3D12(){}
	virtual ~FRHIConstantBuffer_D3D12(){

	}

	D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle() {
		return GpuHandle;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE GetCpuHandle() {
		return CpuHandle;
	}

	D3D12_GPU_VIRTUAL_ADDRESS GetGpuAddress() {
		return GpuVirtualAddress;
	}

	void SetGpuhandle(D3D12_GPU_DESCRIPTOR_HANDLE InHandle) { GpuHandle = InHandle; }
	void SetCpuhandle(D3D12_CPU_DESCRIPTOR_HANDLE InHandle) { CpuHandle = InHandle; }

	void SetGpuVirtualAddress(D3D12_GPU_VIRTUAL_ADDRESS InAddress){GpuVirtualAddress = InAddress;}

	UINT GetRootParameterIndex(){return RootParameterIndex; }
	void SetRootParameterIndex(UINT InIndex){RootParameterIndex = InIndex;}

	virtual void CopyData(int ElementIndex, const T& Data)override;

	std::unique_ptr<FRHIUploadBuffer_D3D12<T>> UploadBuffer;
private:
	UINT RootParameterIndex;

	D3D12_GPU_DESCRIPTOR_HANDLE GpuHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE CpuHandle;
	D3D12_GPU_VIRTUAL_ADDRESS GpuVirtualAddress;
};

template<typename T>
void FRHIConstantBuffer_D3D12<T>::CopyData(int ElementIndex, const T& Data)
{
	UploadBuffer->CopyData(ElementIndex, Data);
}
