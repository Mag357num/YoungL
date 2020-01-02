#include "stdafx.h"
#include "FrameResource_Picking.h"

FrameResource_Picking::FrameResource_Picking(ID3D12Device* device, UINT passCount, UINT objCount, UINT materialCount)
{
	device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&CmdListAlloc));

	PassCB = std::make_unique<UploadBuffer<PassConstants_Picking>>(device, passCount, true);
	MaterialBuffer = std::make_unique< UploadBuffer<MaterialData_Picking>>(device, materialCount, false);
	ObjectCB = std::make_unique<UploadBuffer<ObjectConstants_Picking>>(device, objCount, true);
}

FrameResource_Picking::~FrameResource_Picking()
{

}