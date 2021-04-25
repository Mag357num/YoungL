#pragma once
#include "../RHIColorResource.h"
#include "RHIResource_D3D12.h"

class FRHIColorResource_D3D12 : public FRHIColorResource, public FRHIResource_D3D12
{
public:
	FRHIColorResource_D3D12(int InWidth, int InHeight, EPixelBufferFormat InFormat)
		:FRHIColorResource(InWidth, InHeight, InFormat)
	{}


	virtual ~FRHIColorResource_D3D12() {}

private:

};
