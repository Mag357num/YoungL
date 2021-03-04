#pragma once
#include "../pch.h"


class EsramAllocator
{
public:
	EsramAllocator() {}

	void PushStack();
	void PopStack();

	D3D12_GPU_VIRTUAL_ADDRESS Alloc(size_t size, size_t align, const std::wstring& bufferName)
	{
		(size); (align); (bufferName);
		return D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
	}

	intptr_t SizeOfFreeSpace(void)const
	{
		return 0;
	}
};
