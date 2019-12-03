#include "stdafx.h"
#include "FrameResrouce_Instance.h"

FrameResource_Instance::FrameResource_Instance(ID3D12Device* device, UINT PassCount, UINT MaxInstanceCount, UINT MaterialCount)
{
	ThrowIfFailed(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&cmdListAlloc)));

	PassCB = std::make_unique<UploadBuffer<PassConstant_Instance>>(device, PassCount, true);
	MaterialBuffer = std::make_unique<UploadBuffer<MaterialData_Instance>>(device, MaterialCount, true);
	InstanceBuffer = std::make_unique<UploadBuffer<InstanceData>>(device, MaxInstanceCount, true);
}

FrameResource_Instance::~FrameResource_Instance()
{

}