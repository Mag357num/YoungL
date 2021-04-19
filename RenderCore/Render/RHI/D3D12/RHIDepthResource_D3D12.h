#pragma once
#include "../RHIDepthResource.h"

class FRHIDepthResource_D3D12 : public FRHIDepthResource, public FRHIResource_D3D12
{
public:
	FRHIDepthResource_D3D12(){}
	virtual ~FRHIDepthResource_D3D12(){}

private:

};