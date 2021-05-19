#pragma once


class FRHIColorResource : public IRHIResource
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

		if (UAVHandle)
		{
			delete UAVHandle;
			UAVHandle = nullptr;
		}
	}


	int GetWidth() { return Width; }
	int GetHeight() { return Height; }
	EPixelBufferFormat GetFormat() { return Format; }

	void SetSrvHandle(IRHIResourceHandle* InHandle) { SRVHandle = InHandle; }
	void SetRtvHandle(IRHIResourceHandle* InHandle) { RTVHandle = InHandle; }
	void SetUAVHandle(IRHIResourceHandle* InHandle) { UAVHandle = InHandle; }

	IRHIResourceHandle* GetRTVHandle() { return RTVHandle; }
	IRHIResourceHandle* GetSrvHandle() { return SRVHandle; }
	IRHIResourceHandle* GetUAVHandle() { return UAVHandle; }

protected:
	IRHIResourceHandle* SRVHandle = nullptr;
	IRHIResourceHandle* RTVHandle = nullptr;
	IRHIResourceHandle* UAVHandle = nullptr;

	int Width;
	int Height;

	EPixelBufferFormat Format;
};
