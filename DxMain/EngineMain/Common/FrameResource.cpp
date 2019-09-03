#include "stdafx.h"
#include "FrameResource.h"

FrameResource::FrameResource(ID3D12Device* Device, UINT passCount, UINT objectCount, UINT WaveVertCount)
{
	ThrowIfFailed(Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(CmdListAlloc.GetAddressOf())));

	PassCB = std::make_unique<UploadBuffer<PassConstants>>(Device, passCount, true);
	ObjectCB = std::make_unique<UploadBuffer<ObjectConstants>>(Device, objectCount, true);

	WavesVB = std::make_unique<UploadBuffer<Vertex>>(Device, WaveVertCount, false);
}

FrameResource::~FrameResource()
{

}