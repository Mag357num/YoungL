#pragma once

#include "../RHIContext.h"

class FRHIResource_D3D12 : public IRHIResource
{
public:
	FRHIResource_D3D12(){}
	virtual ~FRHIResource_D3D12(){
		Resource->Release();
		Resource.Reset();
	}

	void Map(UINT SubResource, const D3D12_RANGE* ReadRange, void** Data);
	void UnMap(UINT SubResource, const D3D12_RANGE* WriteRange);

	ComPtr<ID3D12Resource> Resource;
private:
	

};

void FRHIResource_D3D12::Map(UINT SubResource, const D3D12_RANGE* ReadRange, void** Data)
{
	if (Resource)
	{
		Resource->Map(SubResource, ReadRange, Data);
	}
}

void FRHIResource_D3D12::UnMap(UINT SubResource, const D3D12_RANGE* WriteRange)
{
	if (Resource)
	{
		Resource->Unmap(SubResource, WriteRange);
	}
}
