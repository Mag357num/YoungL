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

	void SetSrvHandle(IRHIResourceHandle* InHandle){ SRVHandle = InHandle;}
	void SetDsvHandle(IRHIResourceHandle* InHandle){ DSVHandle = InHandle; }

protected:
	IRHIResourceHandle* SRVHandle;
	IRHIResourceHandle* DSVHandle;
};