#pragma once
#include "../RHIContext.h"
#include "RHIUploadBuffer_D3D12.h"

class FRHIConstantBuffer_D3D12 : public IRHIConstantBuffer
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

	void SetGpuhandle(D3D12_GPU_DESCRIPTOR_HANDLE InHandle) { GpuHandle = InHandle; }
	void SetCpuhandle(D3D12_CPU_DESCRIPTOR_HANDLE InHandle) { CpuHandle = InHandle; }

	UINT GetRootParameterIndex(){return RootParameterIndex; }
	void SetRootParameterIndex(UINT InIndex){RootParameterIndex = InIndex;}

	virtual void CopyData(int ElementIndex, const FObjectConstants& Data);

	std::unique_ptr<FRHIUploadBuffer_D3D12> UploadBuffer;
private:
	UINT RootParameterIndex;

	D3D12_GPU_DESCRIPTOR_HANDLE GpuHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE CpuHandle;
};

void FRHIConstantBuffer_D3D12::CopyData(int ElementIndex, const FObjectConstants& Data)
{
	UploadBuffer->CopyData(ElementIndex, Data);
}
