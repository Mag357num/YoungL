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

void FColorBuffer::Create(const std::wstring& Name, uint32_t InWidth, uint32_t InHeight, uint32_t InNumMips,
	DXGI_FORMAT InFormat, EsramAllocator& Allocator)
{
	Create(Name, InWidth, InHeight, InNumMips, InFormat);
}

void FColorBuffer::Create(const std::wstring& Name, uint32_t InWidth, uint32_t InHeight, uint32_t InNumMips,
	DXGI_FORMAT InFormat, D3D12_GPU_VIRTUAL_ADDRESS VidMemPtr /* = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN */)
{

	NumMipMaps = (InNumMips == 0 ? ComputeNumMips(InWidth, InHeight) : InNumMips);
	D3D12_RESOURCE_FLAGS ResourceFlag = CombinResourceFlags();
	D3D12_RESOURCE_DESC ResourceDesc = DescribleTexture2D(InWidth, InHeight, 1, NumMipMaps, InFormat, ResourceFlag);

	D3D12_CLEAR_VALUE ClearValue;
	ClearValue.Color[0] = ClearColor.R();
	ClearValue.Color[1] = ClearColor.G();
	ClearValue.Color[2] = ClearColor.B();
	ClearValue.Color[3] = ClearColor.A();
	ClearValue.Format = InFormat;

	CreateTextureResource(Graphics::g_Device, Name, ResourceDesc, ClearValue, VidMemPtr);
	CreateDerivedViews(Graphics::g_Device, InFormat, 1, NumMipMaps);
}

void FColorBuffer::CreateArray(const std::wstring& Name, uint32_t InWidth, uint32_t InHeight, uint32_t ArrayCount, DXGI_FORMAT InFormat, EsramAllocator& Allocator)
{
	CreateArray(Name, InWidth, InHeight, ArrayCount, InFormat);
}

void FColorBuffer::CreateArray(const std::wstring& Name, uint32_t InWidth, uint32_t InHeight, uint32_t ArrayCount, DXGI_FORMAT InFormat, D3D12_GPU_VIRTUAL_ADDRESS VidMemPtr /* = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN */)
{
	D3D12_RESOURCE_FLAGS ResourceFlag = CombinResourceFlags();
	D3D12_RESOURCE_DESC ResourceDesc = DescribleTexture2D(InWidth, InHeight, ArrayCount, 1, InFormat, ResourceFlag);

	D3D12_CLEAR_VALUE ClearValue;
	ClearValue.Color[0] = ClearColor.R();
	ClearValue.Color[1] = ClearColor.G();
	ClearValue.Color[2] = ClearColor.B();
	ClearValue.Color[3] = ClearColor.A();

	CreateTextureResource(Graphics::g_Device, Name, ResourceDesc, ClearValue, VidMemPtr);
	CreateDerivedViews(Graphics::g_Device, Format, ArrayCount, 1);
}

void FColorBuffer::CreateDerivedViews(ID3D12Device* Device, DXGI_FORMAT InFormat, uint32_t InArraySize, uint32_t NumMips /* = 1 */)
{
	ASSERT(InArraySize == 1 || NumMips == 1, "We don't spport on auto mips on texture arrays");

	NumMipMaps = NumMips - 1;

	D3D12_RENDER_TARGET_VIEW_DESC RtvDesc = {};
	D3D12_SHADER_RESOURCE_VIEW_DESC SrvDesc = {};
	D3D12_UNORDERED_ACCESS_VIEW_DESC UavDesc = {};

	RtvDesc.Format = InFormat;
	SrvDesc.Format = InFormat;
	UavDesc.Format = GetUAVFormat(InFormat);

	SrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	if (InArraySize > 1)
	{
		RtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
		RtvDesc.Texture2DArray.ArraySize = (UINT)InArraySize;
		RtvDesc.Texture2DArray.MipSlice = 0;
		RtvDesc.Texture2DArray.FirstArraySlice = 0;

		UavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
		UavDesc.Texture2DArray.ArraySize = (UINT)InArraySize;
		UavDesc.Texture2DArray.MipSlice = 0;
		UavDesc.Texture2DArray.FirstArraySlice = 0;

		SrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
		SrvDesc.Texture2DArray.ArraySize = (UINT)InArraySize;
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
	//if (NumMipMaps == 0)
	//	return;

	//FComputeContext& Context = Context.GetComputeContext();

	//Context.SetRootSignature(Graphics::g_CommonRS);

	//Context.TransitionResource(*this, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	//Context.SetDynamicDescriptor(1, 0, m_SRVHandle);

	//for (uint32_t TopMip = 0; TopMip < m_NumMipMaps; )
	//{
	//	uint32_t SrcWidth = m_Width >> TopMip;
	//	uint32_t SrcHeight = m_Height >> TopMip;
	//	uint32_t DstWidth = SrcWidth >> 1;
	//	uint32_t DstHeight = SrcHeight >> 1;

	//	// Determine if the first downsample is more than 2:1.  This happens whenever
	//	// the source width or height is odd.
	//	uint32_t NonPowerOfTwo = (SrcWidth & 1) | (SrcHeight & 1) << 1;
	//	if (m_Format == DXGI_FORMAT_R8G8B8A8_UNORM_SRGB)
	//		Context.SetPipelineState(Graphics::g_GenerateMipsGammaPSO[NonPowerOfTwo]);
	//	else
	//		Context.SetPipelineState(Graphics::g_GenerateMipsLinearPSO[NonPowerOfTwo]);

	//	// We can downsample up to four times, but if the ratio between levels is not
	//	// exactly 2:1, we have to shift our blend weights, which gets complicated or
	//	// expensive.  Maybe we can update the code later to compute sample weights for
	//	// each successive downsample.  We use _BitScanForward to count number of zeros
	//	// in the low bits.  Zeros indicate we can divide by two without truncating.
	//	uint32_t AdditionalMips;
	//	_BitScanForward((unsigned long*)&AdditionalMips,
	//		(DstWidth == 1 ? DstHeight : DstWidth) | (DstHeight == 1 ? DstWidth : DstHeight));
	//	uint32_t NumMips = 1 + (AdditionalMips > 3 ? 3 : AdditionalMips);
	//	if (TopMip + NumMips > m_NumMipMaps)
	//		NumMips = m_NumMipMaps - TopMip;

	//	// These are clamped to 1 after computing additional mips because clamped
	//	// dimensions should not limit us from downsampling multiple times.  (E.g.
	//	// 16x1 -> 8x1 -> 4x1 -> 2x1 -> 1x1.)
	//	if (DstWidth == 0)
	//		DstWidth = 1;
	//	if (DstHeight == 0)
	//		DstHeight = 1;

	//	Context.SetConstants(0, TopMip, NumMips, 1.0f / DstWidth, 1.0f / DstHeight);
	//	Context.SetDynamicDescriptors(2, 0, NumMips, m_UAVHandle + TopMip + 1);
	//	Context.Dispatch2D(DstWidth, DstHeight);

	//	Context.InsertUAVBarrier(*this);

	//	TopMip += NumMips;
	//}

	//Context.TransitionResource(*this, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE |
	//	D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
}