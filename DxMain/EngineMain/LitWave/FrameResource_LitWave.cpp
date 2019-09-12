#include "stdafx.h"
#include "FrameResource_LitWave.h"
#include "../Common/FrameResource.h"

FrameResource_LitWave::FrameResource_LitWave(ID3D12Device* Device, UINT passCount, UINT objectCount, UINT MaterialCount, UINT WaveVertCount)
{
	ThrowIfFailed(Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(CmdListAlloc.GetAddressOf())));

	PassCB = std::make_unique<UploadBuffer<PassConstants_LitWave>>(Device, passCount, true);
	ObjectCB = std::make_unique<UploadBuffer<ObjectConstants>>(Device, objectCount, true);

	WavesVB = std::make_unique<UploadBuffer<Vertex_LitWave>>(Device, WaveVertCount, false);

	MaterialCB = std::make_unique<UploadBuffer<MaterialConstants>>(Device, MaterialCount, true);
}

FrameResource_LitWave::~FrameResource_LitWave()
{

}