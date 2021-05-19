#pragma once
#include "RHIResourceHandle.h"

class FRHIDepthResource : public IRHIResource
{
public:
	FRHIDepthResource(int InWidth, int InHeight, EPixelBufferFormat InFormat)
		:Width(InWidth),
		Height(InHeight),
		Format(InFormat)
	{
	}

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


	int GetWidth(){return Width;}
	int GetHeight(){return Height;}
	EPixelBufferFormat GetFormat(){return Format;}

	void SetSrvHandle(IRHIResourceHandle* InHandle){ SRVHandle = InHandle;}
	void SetDsvHandle(IRHIResourceHandle* InHandle){ DSVHandle = InHandle; }

	IRHIResourceHandle* GetDsvHandle(){return DSVHandle;}
	IRHIResourceHandle* GetSrvHandle() { return SRVHandle; }

protected:
	IRHIResourceHandle* SRVHandle;
	IRHIResourceHandle* DSVHandle;

	int Width;
	int Height;

	EPixelBufferFormat Format;
};
