#include "pch.h"
#include "RHIResource_D3D12.h"


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