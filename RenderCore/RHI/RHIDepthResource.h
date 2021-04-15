#pragma once
#include "RHIResourceHandle.h"

class FRHIDepthResource
{
public:
	FRHIDepthResource(){}
	virtual ~FRHIDepthResource(){
		if (SRVHandle)
		{
			delete SRVHandle;
			SRVHandle = nullptr;
		}

		if (DSVHandle)
		{
			delete DSVHandle;
			DSVHandle = nullptr;
		}
	}

protected:
	IRHIResourceHandle* SRVHandle;
	IRHIResourceHandle* DSVHandle;
};
