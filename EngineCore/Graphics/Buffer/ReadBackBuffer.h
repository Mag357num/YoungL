#pragma once

#include "GPUBuffer.h"

class ReadBackBuffer : public GpuBuffer
{
public:
	virtual ~ReadBackBuffer() { Destroy(); }

	void Create(const std::wstring& Name, uint32_t NumElements, uint32_t ElementSize);

	void* Map(void);
	void Unmap(void);
protected:
	void CreateDerivedViews(void) {}

};

