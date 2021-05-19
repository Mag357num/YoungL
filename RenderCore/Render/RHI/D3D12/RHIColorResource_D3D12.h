#pragma once
#include "../RHIColorResource.h"
#include "RHIResource_D3D12.h"

class FRHIColorResource_D3D12 : public FRHIColorResource
{
public:
	FRHIColorResource_D3D12(int InWidth, int InHeight, EPixelBufferFormat InFormat)
		:FRHIColorResource(InWidth, InHeight, InFormat)
	{}


	virtual ~FRHIColorResource_D3D12() {
		if (Resource != nullptr)
		{
			Resource.Reset();
		}

		if (UploadResource != nullptr)
		{
			UploadResource.Reset();
		}
	
	}

	void SetClearValue(D3D12_CLEAR_VALUE InValue){ClearValue = InValue;}
	D3D12_CLEAR_VALUE GetClearValue(){return ClearValue;}


	void Map(UINT SubResource, const D3D12_RANGE* ReadRange, void** Data);
	void UnMap(UINT SubResource, const D3D12_RANGE* WriteRange);

	ComPtr<ID3D12Resource> Resource;
	ComPtr<ID3D12Resource> UploadResource;


private:
	D3D12_CLEAR_VALUE ClearValue;
};





