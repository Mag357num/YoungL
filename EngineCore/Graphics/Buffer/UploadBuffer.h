#pragma once

#include "GPUResource.h"

class UploadBuffer : public GPUResource
{
public:

	virtual ~UploadBuffer() { Destroy(); }

	void Create(const std::wstring& Name, size_t BufferSize);

	void* Map(void);
	void Unmap(size_t Begin = 0, size_t End = -1);

	size_t GetBufferSize() const { return m_BufferSize; }
private:
	size_t m_BufferSize;
};
