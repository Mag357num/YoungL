#include "pch.h"
#include "GPUDriven.h"

FGPUDriven::FGPUDriven()
{

}

FGPUDriven::~FGPUDriven()
{
	if (FrustumCullResult)
	{
		delete FrustumCullResult;
		FrustumCullResult = nullptr;
	}
}

void FGPUDriven::InitFrustumCull(IRHIContext* RHIContext, FPSOManager* InPSOManager)
{
	//create frustum cull result buffer
	FColorResourceDesc ColorDesc;
	ColorDesc.Width = 64;
	ColorDesc.Height = 64;
	ColorDesc.Format = PixelFormat_R16G16B16A16_Float;
	ColorDesc.ResourceFlag = Resource_Allow_Unordered_Access;
	ColorDesc.ResourceState = State_Uav;

	FrustumCullResult = RHIContext->CreateColorResource(ColorDesc);
	RHIContext->CreateSrvForColorResource(FrustumCullResult, true);


	InPSOManager->CreateFrustumCullPSO(RHIContext, "FrustumPSO");
}