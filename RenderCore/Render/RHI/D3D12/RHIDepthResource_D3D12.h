#pragma once
#include "../RHIDepthResource.h"

class FRHIDepthResource_D3D12 : public FRHIDepthResource
{
public:
	FRHIDepthResource_D3D12(int InWidth, int InHeight, EPixelBufferFormat InFormat)
		:FRHIDepthResource(InWidth, InHeight, InFormat)
	{}


	virtual ~FRHIDepthResource_D3D12(){
	
		if (Resource != nullptr)
		{
			Resource.Reset();
		}

		if (UploadResource != nullptr)
		{
			UploadResource.Reset();
		}

	}



	void Map(UINT SubResource, const D3D12_RANGE* ReadRange, void** Data);
	void UnMap(UINT SubResource, const D3D12_RANGE* WriteRange);

	ComPtr<ID3D12Resource> Resource;
	ComPtr<ID3D12Resource> UploadResource;
private:


};