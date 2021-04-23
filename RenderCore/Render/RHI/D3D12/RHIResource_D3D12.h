#pragma once

#include "../RHIResource.h"
#include <d3d12.h>
#include <wrl.h>
using namespace Microsoft::WRL;

class FRHIResource_D3D12 : public IRHIResource
{
public:
	FRHIResource_D3D12(){}
	virtual ~FRHIResource_D3D12(){
		if (Resource != nullptr)
		{
			Resource.Reset();
		}
		
	}

	void Map(UINT SubResource, const D3D12_RANGE* ReadRange, void** Data);
	void UnMap(UINT SubResource, const D3D12_RANGE* WriteRange);

	ComPtr<ID3D12Resource> Resource;
private:
	

};

