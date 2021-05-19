#include "pch.h"
#include "FrustumCull.h"

void FFrustumCull::InitialFrustumCull(IRHIContext* RHIContext, FPSOManager* InPSOManager)
{
	//create frustum cull result buffer
	FColorResourceDesc ColorDesc;
	ColorDesc.Width = 64;
	ColorDesc.Height = 64;
	ColorDesc.Format = PixelFormat_R16G16B16A16_Float;
	ColorDesc.ResourceFlag = Resource_Allow_Unordered_Access;
	ColorDesc.ResourceState = State_GenerateRead;

	FrustumCullResult = RHIContext->CreateColorResource(ColorDesc);
	RHIContext->CreateSrvForColorResource(FrustumCullResult, true);

	PSOName = "FrustumPSO";
	InPSOManager->CreateFrustumCullPSO(RHIContext, PSOName);
}

void FFrustumCull::Run(IRHIContext* RHIContext, FPSOManager* InPSOManager)
{
	RHIContext->BeginEvent(L"FrustumCull");

	RHIContext->TransitionResource(FrustumCullResult, State_GenerateRead, State_Uav);

	RHIContext->SetComputePipilineState(InPSOManager->GeComputePSO(PSOName));
	RHIContext->SetColorUAV(0, FrustumCullResult);

	RHIContext->DispatchCS(8, 8, 1);

	RHIContext->TransitionResource(FrustumCullResult, State_Uav, State_GenerateRead);

	RHIContext->EndEvent();
}