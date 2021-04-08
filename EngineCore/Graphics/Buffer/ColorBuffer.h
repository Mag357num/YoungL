#pragma once

#include "PixelBuffer.h"
#include "Color.h"
#include "GPUBuffer.h"
//#include "../RHI/CommandContext.h"
#include "GPUResource.h"

class EsramAllocator;

class FColorBuffer : public FPixelBuffer
{
public:

	FColorBuffer(FColor InClearColor = FColor(0.0f, 1.0f, 0.0f, 1.0f)) :
		ClearColor(InClearColor),
		NumMipMaps(0),
		FragmentCount(1),
		SampleCount(1)
	{
		RtvHandle.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
		SrvHandle.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;

		for (int i =0; i< _countof(UavHandle); ++i)
		{
			UavHandle[i].ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
		}
	}

	// Create a color buffer from a swap chain buffer.  Unordered access is restricted.
	void CreateFromSwapChain(const std::wstring& Name, ID3D12Resource* ResourceFromSwapchain);

	// Create a color buffer.  If an address is supplied, memory will not be allocated.
	// The vmem address allows you to alias buffers (which can be especially useful for
	// reusing ESRAM across a frame.)
	void Create(const std::wstring& Name, uint32_t InWidth, uint32_t InHeight, uint32_t InNumMips,
		DXGI_FORMAT InFormat, D3D12_GPU_VIRTUAL_ADDRESS VidMemPtr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN);

	// Create a color buffer.  Memory will be allocated in ESRAM (on Xbox One).  On Windows,
   // this functions the same as Create() without a video address.
	void Create(const std::wstring& Name, uint32_t InWidth, uint32_t InHeight, uint32_t InNumMips,
		DXGI_FORMAT InFormat, EsramAllocator& Allocator);

	// Create a color buffer.  If an address is supplied, memory will not be allocated.
	// The vmem address allows you to alias buffers (which can be especially useful for
	// reusing ESRAM across a frame.)
	void CreateArray(const std::wstring& Name, uint32_t InWidth, uint32_t InHeight, uint32_t ArrayCount,
		DXGI_FORMAT InFormat, D3D12_GPU_VIRTUAL_ADDRESS VidMemPtr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN);

	// Create a color buffer.  Memory will be allocated in ESRAM (on Xbox One).  On Windows,
   // this functions the same as Create() without a video address.
	void CreateArray(const std::wstring& Name, uint32_t InWidth, uint32_t InHeight, uint32_t ArrayCount,
		DXGI_FORMAT InFormat, EsramAllocator& Allocator);


	//get pre created cpu visible descriptor handles
	const D3D12_CPU_DESCRIPTOR_HANDLE& GetSrv(void) const { return SrvHandle; }
	const D3D12_CPU_DESCRIPTOR_HANDLE& GetRtv(void) const { return RtvHandle; }
	const D3D12_CPU_DESCRIPTOR_HANDLE& GetUav(void) const { return UavHandle[0]; }

	void SetClearColor(FColor InClearColor) { ClearColor = InClearColor; }
	FColor GetClearColor(void) const { return ClearColor; }
	
	void SetMsaaMode(uint32_t NumColorSamples, uint32_t NumCoverageSamples)
	{
		ASSERT(NumCoverageSamples >= NumColorSamples);
		FragmentCount = NumColorSamples;
		SampleCount = NumCoverageSamples;
	}

	// This will work for all texture sizes, but it's recommended for speed and quality
	// that you use dimensions with powers of two (but not necessarily square.)  Pass
	// 0 for ArrayCount to reserve space for mips at creation time.
	void GenerateMipMaps(FCommandContext& Context);

protected:

	D3D12_RESOURCE_FLAGS CombinResourceFlags(void) const
	{
		D3D12_RESOURCE_FLAGS Flags = D3D12_RESOURCE_FLAG_NONE;
		if (Flags == D3D12_RESOURCE_FLAG_NONE && FragmentCount == 1)
		{
			Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		}

		return D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | Flags;
	}

	// Compute the number of texture levels needed to reduce to 1x1.  This uses
	// _BitScanReverse to find the highest set bit.  Each dimension reduces by
	// half and truncates bits.  The dimension 256 (0x100) has 9 mip levels, same
	// as the dimension 511 (0x1FF).
	static inline uint32_t ComputeNumMips(uint32_t InWidth, uint32_t InHeight)
	{
		uint32_t HeighBit;
		_BitScanReverse((unsigned long*)&HeighBit, InWidth | InHeight);
		return HeighBit + 1;
	}

	void CreateDerivedViews(ID3D12Device* Device, DXGI_FORMAT InFormat, uint32_t InArraySize, uint32_t NumMips = 1);

	FColor ClearColor;
	D3D12_CPU_DESCRIPTOR_HANDLE SrvHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE RtvHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE UavHandle[12];
	uint32_t NumMipMaps;
	uint32_t FragmentCount;
	uint32_t SampleCount;


};