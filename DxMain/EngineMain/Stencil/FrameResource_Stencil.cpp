#include "stdafx.h"
#include "FrameResource_Stencil.h"

FrameResource_Stencil::FrameResource_Stencil(ID3D12Device* device, UINT passCount, UINT objectCount, UINT materialCount)
{
	ThrowIfFailed(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&CmdListAlloc)));

	PassCB = std::make_unique<UploadBuffer<PassConstants_Stencil>>(device, passCount, true);
	MaterialCB = std::make_unique<UploadBuffer<MaterialConstants>>(device, materialCount, true);
	ObjectCB = std::make_unique<UploadBuffer<ObjectConstans_Stencil>>(device, objectCount, true);
}

FrameResource_Stencil::~FrameResource_Stencil()
{

}