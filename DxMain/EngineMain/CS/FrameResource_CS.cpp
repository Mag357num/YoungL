#include "stdafx.h"
#include "FrameResource_CS.h"

FrameResource_CS::FrameResource_CS(ID3D12Device* Device, UINT passCount, UINT objectCount, UINT MaterialCount)
{
	ThrowIfFailed(Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(CmdListAlloc.GetAddressOf())));

	if (passCount > 0)
	{
		PassCB = std::make_unique<UploadBuffer<PassConstants_CS>>(Device, passCount, true);
	}

	if (objectCount > 0)
	{
		ObjectCB = std::make_unique<UploadBuffer<ObjectConstants_CS>>(Device, objectCount, true);
	}

	if (MaterialCount >0)
	{
		MaterialCB = std::make_unique<UploadBuffer<MaterialConstants>>(Device, MaterialCount, true);
	}
}

FrameResource_CS::~FrameResource_CS()
{

}