#pragma once
#include "../RHIDepthResource.h"

class FRHIDepthResource_D3D12 : public FRHIDepthResource, public FRHIResource_D3D12
{
public:
	FRHIDepthResource_D3D12(int InWidth, int InHeight, EPixelBufferFormat InFormat)
		:FRHIDepthResource(InWidth, InHeight, InFormat)
	{}


	virtual ~FRHIDepthResource_D3D12(){}

private:

};