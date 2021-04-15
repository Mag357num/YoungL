#pragma once
#include "../RHIResourceHandle.h"

class FRHIResourceHandle_D3D12 : public IRHIResourceHandle
{
public:
	FRHIResourceHandle_D3D12(){}
	virtual ~FRHIResourceHandle_D3D12(){}

	D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle() {
		return GpuHandle;
		}

	D3D12_CPU_DESCRIPTOR_HANDLE GetCpuHandle() {
		return CpuHandle;
	}

	void SetGpuhandle(D3D12_GPU_DESCRIPTOR_HANDLE InHandle){ GpuHandle  = InHandle;}
	void SetCpuhandle(D3D12_CPU_DESCRIPTOR_HANDLE InHandle){CpuHandle = InHandle;}

private:
	D3D12_GPU_DESCRIPTOR_HANDLE GpuHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE CpuHandle;
};