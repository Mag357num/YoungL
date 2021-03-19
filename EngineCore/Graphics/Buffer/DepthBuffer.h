#pragma once
#include "PixelBuffer.h"

class DepthBuffer : public PixelBuffer
{
public:
	DepthBuffer(float ClearDepth = 0.0f, uint8_t ClearStencil = 0)
		:Y_ClearDepth(ClearDepth),
		Y_ClearStencil(ClearStencil)
	{
		Y_Hdsv[0].ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
		Y_Hdsv[1].ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
		Y_Hdsv[2].ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
		Y_Hdsv[3].ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;

		Y_hDepthSrv.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
		Y_hStencilSrv.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
	}

	// Create a depth buffer.  If an address is supplied, memory will not be allocated.
	// The vmem address allows you to alias buffers (which can be especially useful for
	// reusing ESRAM across a frame.)
	void Create(const std::wstring& Name, uint32_t Width, uint32_t Height, DXGI_FORMAT Format,
		D3D12_GPU_VIRTUAL_ADDRESS VidMemPtr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN);

	// Create a depth buffer.  Memory will be allocated in ESRAM (on Xbox One).  On Windows,
	// this functions the same as Create() without a video address.
	void Create(const std::wstring& Name, uint32_t Width, uint32_t Height, DXGI_FORMAT Format,
		EsramAllocator& Allocator);


	void Create(const std::wstring& Name, uint32_t Width, uint32_t Height, uint32_t NumSamples, DXGI_FORMAT Format,
		D3D12_GPU_VIRTUAL_ADDRESS VidMemPtr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN);
	void Create(const std::wstring& Name, uint32_t Width, uint32_t Height, uint32_t NumSamples, DXGI_FORMAT Format,
		EsramAllocator& Allocator);


	const D3D12_CPU_DESCRIPTOR_HANDLE& GetDsv() const { return Y_Hdsv[0]; }
	const D3D12_CPU_DESCRIPTOR_HANDLE& GetDsv_DepthReadOnly() const { return Y_Hdsv[1]; }
	const D3D12_CPU_DESCRIPTOR_HANDLE& GetDsv_StencilReadOnly() const { return Y_Hdsv[2]; }
	const D3D12_CPU_DESCRIPTOR_HANDLE& GetDsv_ReadOnly() const { return Y_Hdsv[3]; }

	const D3D12_CPU_DESCRIPTOR_HANDLE& GetDepthSrv() const { return Y_hDepthSrv; }
	const D3D12_CPU_DESCRIPTOR_HANDLE& GetStencilSrv() const { return Y_hStencilSrv; }

	float GetClearDepth(void) const { return Y_ClearDepth; }
	uint8_t GetClearStencil(void) const { return Y_ClearStencil; }
	

protected:
	void CreateDerivedViews(ID3D12Device* Device, DXGI_FORMAT Format);

	float Y_ClearDepth;
	uint8_t Y_ClearStencil;

	D3D12_CPU_DESCRIPTOR_HANDLE Y_Hdsv[4];
	D3D12_CPU_DESCRIPTOR_HANDLE Y_hDepthSrv;
	D3D12_CPU_DESCRIPTOR_HANDLE Y_hStencilSrv;
};

