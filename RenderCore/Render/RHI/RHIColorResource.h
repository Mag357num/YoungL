#pragma once

class FRHIColorResource: public IRHIResource
{
public:
	FRHIColorResource(int InWidth, int InHeight, EPixelBufferFormat InFormat)
		:Width(InWidth),
		Height(InHeight),
		Format(InFormat)
	{
	}

	virtual ~FRHIColorResource() {
		if (SRVHandle)
		{
			delete SRVHandle;
			SRVHandle = nullptr;
		}

		if (RTVHandle)
		{
			delete RTVHandle;
			RTVHandle = nullptr;
		}
	}


	int GetWidth() { return Width; }
	int GetHeight() { return Height; }
	EPixelBufferFormat GetFormat() { return Format; }

	void SetSrvHandle(IRHIResourceHandle* InHandle) { SRVHandle = InHandle; }
	void SetRtvHandle(IRHIResourceHandle* InHandle) { RTVHandle = InHandle; }

	IRHIResourceHandle* GetRTVHandle() { return RTVHandle; }
	IRHIResourceHandle* GetSrvHandle() { return SRVHandle; }

protected:
	IRHIResourceHandle* SRVHandle;
	IRHIResourceHandle* RTVHandle;

	int Width;
	int Height;

	EPixelBufferFormat Format;
};
