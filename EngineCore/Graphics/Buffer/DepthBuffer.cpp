#include "DepthBuffer.h"

#include "../GraphicsCore.h"

void DepthBuffer::CreateDerivedViews(ID3D12Device* Device, DXGI_FORMAT Format)
{
	ID3D12Resource* Resource = Y_Resource.Get();

	D3D12_DEPTH_STENCIL_VIEW_DESC DsvDesc;

	DsvDesc.Format = GetDSVFormat(Format);
	if (Resource->GetDesc().SampleDesc.Count ==1)
	{
		DsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		DsvDesc.Texture2D.MipSlice = 0;
	}
	else
	{
		DsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;
	}


	if (Y_Hdsv[0].ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
	{
		Y_Hdsv[0].ptr = Graphics.AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		Y_Hdsv[2].ptr = Graphics.AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	}

	DsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	Device->CreateDepthStencilView(Resource, &DsvDesc, Y_Hdsv[0]);

	DsvDesc.Flags = D3D12_DSV_FLAG_READ_ONLY_DEPTH;
	Device->CreateDepthStencilView(Resource, &DsvDesc, Y_Hdsv[1]);

	DXGI_FORMAT StencilFormat = GetStencilFormat(Format);
	if (StencilFormat != DXGI_FORMAT_UNKNOWN)
	{
		if (Y_Hdsv[2] == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
		{
			Y_Hdsv[2].ptr = Graphics::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
			Y_Hdsv[3].ptr = Graphics::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		}

		DsvDesc.Flags = D3D12_DSV_FLAG_READ_ONLY_STENCIL;
		Device->CreateDepthStencilView(Resource, &DsvDesc, Y_Hdsv[2]);

		DsvDesc.Flags = D3D12_DSV_FLAG_READ_ONLY_DEPTH | D3D12_DSV_FLAG_READ_ONLY_STENCIL;
		Device->CreateDepthStencilView(Resource, &DsvDesc, Y_Hdsv[3]);
	}
	else
	{
		Y_Hdsv[2] = Y_Hdsv[0];
		Y_Hdsv[3] = Y_Hdsv[1];
	}

	D3D12_SHADER_RESOURCE_VIEW_DESC SrvDesc;
	SrvDesc.Format = GetDepthFormat(Format);
	if (DsvDesc.ViewDimension == D3D12_DSV_DIMENSION_TEXTURE2D)
	{
		SrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	}
	else
	{
		SrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
	}

	SrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	Device->CreateShaderResourceView(Resource, &SrvDesc, Y_hDepthSrv);

	if (StencilFormat != DXGI_FORMAT_UNKNOWN)
	{
		if (Y_hStencilSrv.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
		{
			Y_hStencilSrv.ptr = Graphics::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		}

		SrvDesc.Format = StencilFormat;
		Device->CreateShaderResourceView(Resource, &SrvDesc, Y_hStencilSrv);
	}

}

void DepthBuffer::Create(const std::wstring& Name, uint32_t Width, uint32_t Height, uint32_t NumSamples, DXGI_FORMAT Format, EsramAllocator& Allocator)
{
	Create(Name, Width, Height, NumSamples, Format);
}

void DepthBuffer::Create(const std::wstring& Name, uint32_t Width, uint32_t Height, uint32_t NumSamples, DXGI_FORMAT Format, D3D12_GPU_VIRTUAL_ADDRESS VidMemPtr /* = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN */)
{
	D3D12_RESOURCE_DESC ResoruceDesc = DescribleTexture2D(Width, Height, 1, 1, Format, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
	ResoruceDesc.SampleDesc.Count = NumSamples;

	D3D12_CLEAR_VALUE ClearValue = {};
	ClearValue.Format = Format;
	CreateTextureResource(Graphics::g_Device, Name, &ResoruceDesc, ClearValue, VidMemPtr);
	CreateDerivedViews(Graphics::g_Device, Format);
}

void DepthBuffer::Create(const std::wstring& Name, uint32_t Width, uint32_t Height, DXGI_FORMAT Format, EsramAllocator& Allocator)
{
	Create(Name, Width, Height, 1, Format, Allocator);
}

void DepthBuffer::Create(const std::wstring& Name, uint32_t Width, uint32_t Height, DXGI_FORMAT Format, D3D12_GPU_VIRTUAL_ADDRESS VidMemPtr /* = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN */)
{
	Create(Name, Width, Height, Samples, Format);
}
