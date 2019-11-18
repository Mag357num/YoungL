#include "stdafx.h"
#include "FrameResource_Tessellation.h"

FrameResource_Tessellation::FrameResource_Tessellation(ID3D12Device* mDevice, UINT passCount, UINT objCount, UINT materialCount)
{
	mDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&cmdListAlloc));

	FrameCB = std::make_unique<UploadBuffer<PassConstants_Tessellation>>(mDevice, passCount, true);
	ObjectCB = std::make_unique<UploadBuffer<ObjectConstans_Tessellation>>(mDevice, objCount, true);
	MaterialCB = std::make_unique<UploadBuffer<MaterialConstants>>(mDevice, materialCount, true);
}

FrameResource_Tessellation::~FrameResource_Tessellation()
{

}