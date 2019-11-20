#include "stdafx.h"
#include "FrameResource_CameraDyIndex.h"

FrameResource_CameraDyIndex::FrameResource_CameraDyIndex(ID3D12Device* device, UINT passCount, UINT objectCount, UINT materialCount)
{
	ThrowIfFailed(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&CmdListAlloc)));

	PassCB = std::make_unique<UploadBuffer<PassConstants_CameraDyIndex>>(device, passCount, true);
	MaterialBuffer = std::make_unique<UploadBuffer<MaterialData_CameraDyindex>>(device, materialCount, false);
	ObjectCB = std::make_unique<UploadBuffer<ObjectConstants_CameraDyIndex>>(device, objectCount, true);
}

FrameResource_CameraDyIndex::~FrameResource_CameraDyIndex()
{

}