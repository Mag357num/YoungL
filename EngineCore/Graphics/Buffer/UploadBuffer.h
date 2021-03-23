#pragma once
#include "../../pch.h"
#include "GPUResource.h"

class FUploadBuffer : public FGPUResource
{
public:

	virtual ~FUploadBuffer() { Destroy(); }

	void Create(const std::wstring& Name, size_t BufferSize);

	void* Map(void);
	void Unmap(size_t Begin = 0, size_t End = -1);

	size_t GetBufferSize() const { return BufferSize; }
private:
	size_t BufferSize;
};
