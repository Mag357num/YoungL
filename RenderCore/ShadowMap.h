#pragma once

#include "RHI/RHIDepthResource.h"

class FShadowMap
{
public:
	FShadowMap(int InWidth, int InHeight)
		:ShadowVP(0.0f, 0.0f, (float)InWidth, (float)InHeight)
	{}
	
	~FShadowMap(){
		if (DepthResource)
		{
			delete DepthResource;
			DepthResource = nullptr;
		}
	}

	void SetDepthResource(FRHIDepthResource* InDepthRes){DepthResource = InDepthRes;}

private:
	FViewport ShadowVP;
	FRHIDepthResource* DepthResource;
};
