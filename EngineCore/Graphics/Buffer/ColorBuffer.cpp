#include "pch.h"
#include "ColorBuffer.h"
#include "../RHI/CommandContext.h"
#include "../../Graphics/GraphicsCore.h"

using namespace Graphics;

void FColorBuffer::CreateFromSwapChain(const std::wstring& Name, ID3D12Resource* ResourceFromSwapchain)
{
	AssociateWithResource(Graphics::g_Device, Name, ResourceFromSwapchain, D3D12_RESOURCE_STATE_PRESENT);
	RtvHandle = Graphics::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	Graphics::g_Device->CreateRenderTargetView(GetResource(), nullptr, RtvHandle);
}

void FColorBuffer::Create(const std::wstring& Name, uint32_t Width, uint32_t Height, uint32_t NumMips, DXGI_FORMAT Format, EsramAllocator& Allocator)
{
	Create(Name, Width, Height, NumMips, Format);
}

void FColorBuffer::Create(const std::wstring& Name, uint32_t Width, uint32_t Height, uint32_t NumMips, DXGI_FORMAT Format, D3D12_GPU_VIRTUAL_ADDRESS VidMemPtr /* = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN */)
{

	NumMips = (NumMips == 0 ? ComputeNumMips(Width, Height) : NumMips);
	D3D12_RESOURCE_FLAGS ResourceFlag = CombinResourceFlags();
	D3D12_RESOURCE_DESC ResourceDesc = DescribleTexture2D(Width, Height, 1, NumMips, Format, ResourceFlag);

	D3D12_CLEAR_VALUE ClearValue;
	ClearValue.Color[0] = ClearColor.R();
	ClearValue.Color[1] = ClearColor.G();
	ClearValue.Color[2] = ClearColor.B();
	ClearValue.Color[3] = ClearColor.A();
	ClearValue.Format = Format;

	CreateTextureResource(Graphics::g_Device, Name, ResourceDesc, ClearValue, VidMemPtr);
	CreateDerivedViews(Graphics::g_Device, Format, 1, NumMips);
}

void FColorBuffer::CreateArray(const std::wstring& Name, uint32_t Width, uint32_t Height, uint32_t ArrayCount, DXGI_FORMAT Format, EsramAllocator& Allocator)
{
	CreateArray(Name, Width, Height, ArrayCount, Format);
}

void FColorBuffer::CreateArray(const std::wstring& Name, uint32_t Width, uint32_t Height, uint32_t ArrayCount, DXGI_FORMAT Format, D3D12_GPU_VIRTUAL_ADDRESS VidMemPtr /* = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN */)
{
	D3D12_RESOURCE_FLAGS ResourceFlag = CombinResourceFlags();
	D3D12_RESOURCE_DESC ResourceDesc = DescribleTexture2D(Width, Height, ArrayCount, 1, Format, ResourceFlag);

	D3D12_CLEAR_VALUE ClearValue;
	ClearValue.Color[0] = ClearColor.R();
	ClearValue.Color[1] = ClearColor.G();
	ClearValue.Color[2] = ClearColor.B();
	ClearValue.Color[3] = ClearColor.A();

	CreateTextureResource(Graphics::g_Device, Name, ResourceDesc, ClearValue, VidMemPtr);
	CreateDerivedViews(Graphics::g_Device, Format, ArrayCount, 1);
}

void FColorBuffer::CreateDerivedViews(ID3D12Device* Device, DXGI_FORMAT Format, uint32_t ArraySize, uint32_t NumMips /* = 1 */)
{
	ASSERT(ArraySize == 1 || NumMips == 1, "We don't spport on auto mips on texture arrays");

	NumMipMaps = NumMips - 1;

	D3D12_RENDER_TARGET_VIEW_DESC RtvDesc = {};
	D3D12_SHADER_RESOURCE_VIEW_DESC SrvDesc = {};
	D3D12_UNORDERED_ACCESS_VIEW_DESC UavDesc = {};

	RtvDesc.Format = Format;
	SrvDesc.Format = Format;
	UavDesc.Format = GetUAVFormat(Format);

	SrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	if (ArraySize > 1)
	{
		RtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
		RtvDesc.Texture2DArray.ArraySize = (UINT)ArraySize;
		RtvDesc.Texture2DArray.MipSlice = 0;
		RtvDesc.Texture2DArray.FirstArraySlice = 0;

		UavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
		UavDesc.Texture2DArray.ArraySize = (UINT)ArraySize;
		UavDesc.Texture2DArray.MipSlice = 0;
		UavDesc.Texture2DArray.FirstArraySlice = 0;

		SrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
		SrvDesc.Texture2DArray.ArraySize = (UINT)ArraySize;
		SrvDesc.Texture2DArray.FirstArraySlice = 0;
		SrvDesc.Texture2DArray.MipLevels = NumMips;
		SrvDesc.Texture2DArray.MostDetailedMip = 0;
	}
	else if (FragmentCount > 1)
	{
		RtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;
		SrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY;
	}
	else
	{
		RtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		RtvDesc.Texture2D.MipSlice = 0;

		UavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
		UavDesc.Texture2D.MipSlice = 0;

		SrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		SrvDesc.Texture2D.MipLevels = NumMips;
		SrvDesc.Texture2D.MostDetailedMip = 0;
	}

	if (SrvHandle.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
	{
		RtvHandle = Graphics::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		SrvHandle = Graphics::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}

	ID3D12Resource* TempResource = Resource.Get();

	Device->CreateRenderTargetView(TempResource, &RtvDesc, RtvHandle);

	Device->CreateShaderResourceView(TempResource, &SrvDesc, SrvHandle);

	//create uav for each mip level
	for (uint32_t i=0; i< NumMips; ++i)
	{
		if (UavHandle[i].ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
		{
			UavHandle[i] = Graphics::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		}

		Device->CreateUnorderedAccessView(TempResource, nullptr, &UavDesc, UavHandle[i]);

		UavDesc.Texture2D.MipSlice++;
	}

}

void FColorBuffer::GenerateMipMaps(FCommandContext& Context)
{

}