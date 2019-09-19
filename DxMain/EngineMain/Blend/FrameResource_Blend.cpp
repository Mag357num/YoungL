#include "stdafx.h"
#include "FrameResource_Blend.h"

FrameResource_Blend::FrameResource_Blend(ID3D12Device* Device, UINT passCount, UINT objectCount, UINT MaterialCount, UINT WaveVertCount)
{
	ThrowIfFailed(Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(CmdListAlloc.GetAddressOf())));

	PassCB = std::make_unique<UploadBuffer<PassConstants_Blend>>(Device, passCount, true);
	ObjectCB = std::make_unique<UploadBuffer<ObjectConstants_Blend>>(Device, objectCount, true);

	if (WaveVertCount > 0)
	{
		WavesVB = std::make_unique<UploadBuffer<Vertex_Blend>>(Device, WaveVertCount, false);
	}


	MaterialCB = std::make_unique<UploadBuffer<MaterialConstants>>(Device, MaterialCount, true);
}

FrameResource_Blend::~FrameResource_Blend()
{

}