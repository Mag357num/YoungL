#pragma once
#include "PixelBuffer.h"

class FDepthBuffer : public FPixelBuffer
{
public:
	FDepthBuffer(float ClearDepth = 0.0f, uint8_t ClearStencil = 0)
		:ClearDepth(ClearDepth),
		ClearStencil(ClearStencil)
	{
		Hdsv[0].ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
		Hdsv[1].ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
		Hdsv[2].ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
		Hdsv[3].ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;

		HDepthSrv.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
		HStencilSrv.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
	}

	// Create a depth buffer.  If an address is supplied, memory will not be allocated.
	// The vmem address allows you to alias buffers (which can be especially useful for
	// reusing ESRAM across a frame.)
	void Create(const std::wstring& Name, uint32_t InWidth, uint32_t InHeight, DXGI_FORMAT InFormat,
		D3D12_GPU_VIRTUAL_ADDRESS VidMemPtr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN);

	// Create a depth buffer.  Memory will be allocated in ESRAM (on Xbox One).  On Windows,
	// this functions the same as Create() without a video address.
	void Create(const std::wstring& Name, uint32_t InWidth, uint32_t InHeight, DXGI_FORMAT InFormat,
		EsramAllocator& Allocator);


	void Create(const std::wstring& Name, uint32_t InWidth, uint32_t InHeight, uint32_t NumSamples, DXGI_FORMAT InFormat,
		D3D12_GPU_VIRTUAL_ADDRESS VidMemPtr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN);
	void Create(const std::wstring& Name, uint32_t InWidth, uint32_t InHeight, uint32_t NumSamples, DXGI_FORMAT InFormat,
		EsramAllocator& Allocator);


	const D3D12_CPU_DESCRIPTOR_HANDLE& GetDsv() const { return Hdsv[0]; }
	const D3D12_CPU_DESCRIPTOR_HANDLE& GetDsv_DepthReadOnly() const { return Hdsv[1]; }
	const D3D12_CPU_DESCRIPTOR_HANDLE& GetDsv_StencilReadOnly() const { return Hdsv[2]; }
	const D3D12_CPU_DESCRIPTOR_HANDLE& GetDsv_ReadOnly() const { return Hdsv[3]; }

	const D3D12_CPU_DESCRIPTOR_HANDLE& GetDepthSrv() const { return HDepthSrv; }
	const D3D12_CPU_DESCRIPTOR_HANDLE& GetStencilSrv() const { return HStencilSrv; }

	float GetClearDepth(void) const { return ClearDepth; }
	uint8_t GetClearStencil(void) const { return ClearStencil; }
	

protected:
	void CreateDerivedViews(ID3D12Device* Device, DXGI_FORMAT InFormat);

	float ClearDepth;
	uint8_t ClearStencil;

	D3D12_CPU_DESCRIPTOR_HANDLE Hdsv[4];
	D3D12_CPU_DESCRIPTOR_HANDLE HDepthSrv;
	D3D12_CPU_DESCRIPTOR_HANDLE HStencilSrv;
};

