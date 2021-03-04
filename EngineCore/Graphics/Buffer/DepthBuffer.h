#pragma once
#include "PixelBuffer.h"

class DepthBuffer : public PixelBuffer
{
public:
	DepthBuffer(float ClearDepth = 0.0f, uint8_t ClearStencil = 0)
		:m_ClearDepth(ClearDepth),
		m_ClearStencil(ClearStencil)
	{
		m_Hdsv[0].ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
		m_Hdsv[1].ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
		m_Hdsv[2].ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
		m_Hdsv[3].ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;

		m_hDepthSrv.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
		m_hStencilSrv.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
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


	const D3D12_CPU_DESCRIPTOR_HANDLE& GetDsv() const { return m_Hdsv[0]; }
	const D3D12_CPU_DESCRIPTOR_HANDLE& GetDsv_DepthReadOnly() const { return m_Hdsv[1]; }
	const D3D12_CPU_DESCRIPTOR_HANDLE& GetDsv_StencilReadOnly() const { return m_Hdsv[2]; }
	const D3D12_CPU_DESCRIPTOR_HANDLE& GetDsv_ReadOnly() const { return m_Hdsv[3]; }

	const D3D12_CPU_DESCRIPTOR_HANDLE& GetDepthSrv() const { return m_hDepthSrv; }
	const D3D12_CPU_DESCRIPTOR_HANDLE& GetStencilSrv() const { return m_hStencilSrv; }

	float GetClearDepth(void) const { return m_ClearDepth; }
	uint8_t GetClearStencil(void) const { return m_ClearStencil; }
	

protected:
	void CreateDerivedViewsd(ID3D12Device* Device, DXGI_FORMAT Format);

	float m_ClearDepth;
	uint8_t m_ClearStencil;

	D3D12_CPU_DESCRIPTOR_HANDLE m_Hdsv[4];
	D3D12_CPU_DESCRIPTOR_HANDLE m_hDepthSrv;
	D3D12_CPU_DESCRIPTOR_HANDLE m_hStencilSrv;
};

