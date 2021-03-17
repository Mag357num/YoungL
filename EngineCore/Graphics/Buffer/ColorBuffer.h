#pragma once

#include "PixelBuffer.h"
#include "Color.h"
#include "GPUBuffer.h"

class EsramAllocator;

class ColorBuffer : public PixelBuffer
{
public:

	ColorBuffer(Color ClearColor = Color(0.0f, 0.0f, 0.0f, 0.0f)) :
		m_ClearColor(ClearColor),
		m_NumMipMaps(0),
		m_FragmentCount(1),
		m_SampleCount(1)
	{
		m_RtvHandle.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
		m_SrvHandle.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;

		for (int i =0; i< _countof(m_UavHandle); ++i)
		{
			m_UavHandle[i].ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
		}
	}

	// Create a color buffer from a swap chain buffer.  Unordered access is restricted.
	void CreateFromSwapChain(const std::wstring& Name, ID3D12Resource* ResourceFromSwapchain);

	// Create a color buffer.  If an address is supplied, memory will not be allocated.
	// The vmem address allows you to alias buffers (which can be especially useful for
	// reusing ESRAM across a frame.)
	void Create(const std::wstring& Name, uint32_t Width, uint32_t Height, uint32_t NumMips,
		DXGI_FORMAT Format, D3D12_GPU_VIRTUAL_ADDRESS VidMemPtr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN);

	// Create a color buffer.  Memory will be allocated in ESRAM (on Xbox One).  On Windows,
   // this functions the same as Create() without a video address.
	void Create(const std::wstring& Name, uint32_t Width, uint32_t Height, uint32_t NumMips,
		DXGI_FORMAT Format, EsramAllocator& Allocator);

	// Create a color buffer.  If an address is supplied, memory will not be allocated.
	// The vmem address allows you to alias buffers (which can be especially useful for
	// reusing ESRAM across a frame.)
	void CreateArray(const std::wstring& Name, uint32_t Width, uint32_t Height, uint32_t ArrayCount,
		DXGI_FORMAT Format, D3D12_GPU_VIRTUAL_ADDRESS VidMemPtr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN);

	// Create a color buffer.  Memory will be allocated in ESRAM (on Xbox One).  On Windows,
   // this functions the same as Create() without a video address.
	void CreateArray(const std::wstring& Name, uint32_t Width, uint32_t Height, uint32_t ArrayCount,
		DXGI_FORMAT Format, EsramAllocator& Allocator);


	//get pre created cpu visible descriptor handles
	const D3D12_CPU_DESCRIPTOR_HANDLE& GetSrv(void) const { return m_SrvHandle; }
	const D3D12_CPU_DESCRIPTOR_HANDLE& GetRtv(void) const { return m_RtvHandle; }
	const D3D12_CPU_DESCRIPTOR_HANDLE& GetUav(void) const { return m_UavHandle[0]; }

	void SetClearColor(Color ClearColor) { m_ClearColor = ClearColor; }
	Color GetClearColor(void) const { return m_ClearColor; }
	
	void SetMsaaMode(uint32_t NumColorSamples, uint32_t NumCoverageSamples)
	{
		ASSERT(NumCoverageSamples >= NumColorSamples);
		m_FragmentCount = NumColorSamples;
		m_SampleCount = NumCoverageSamples;
	}

	// This will work for all texture sizes, but it's recommended for speed and quality
	// that you use dimensions with powers of two (but not necessarily square.)  Pass
	// 0 for ArrayCount to reserve space for mips at creation time.
	void GenerateMipMaps(CommandContext& Context);

protected:

	D3D12_RESOURCE_FLAGS CombinResourceFlags(void) const
	{
		D3D12_RESOURCE_FLAGS Flags = D3D12_RESOURCE_FLAG_NONE;
		if (Flags == D3D12_RESOURCE_FLAG_NONE && m_FragmentCount == 1)
		{
			Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		}

		return D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | Flags;
	}

	// Compute the number of texture levels needed to reduce to 1x1.  This uses
	// _BitScanReverse to find the highest set bit.  Each dimension reduces by
	// half and truncates bits.  The dimension 256 (0x100) has 9 mip levels, same
	// as the dimension 511 (0x1FF).
	static inline uint32_t ComputeNumMips(uint32_t Width, uint32_t Height)
	{
		uint32_t HeighBit;
		_BitScanReverse((unsigned long*)&HeighBit, Width | Height);
		return HeighBit + 1;
	}

	void CreateDerivedViews(ID3D12Device* Device, DXGI_FORMAT Format, uint32_t ArraySize, uint32_t NumMips = 1);

	Color m_ClearColor;
	D3D12_CPU_DESCRIPTOR_HANDLE m_SrvHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE m_RtvHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE m_UavHandle[12];
	uint32_t m_NumMipMaps;
	uint32_t m_FragmentCount;
	uint32_t m_SampleCount;


};