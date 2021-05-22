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

#ifdef ENABLE_COMPUTE_PIPELINE
	RHIContext->Compute_BeginEvent(L"FrustumCull");

	RHIContext->Compute_TransitionResource(FrustumCullResult, State_GenerateRead, State_Uav);

	RHIContext->Compute_SetPipilineState(InPSOManager->GeComputePSO(PSOName));
	RHIContext->Compute_SetColorUAV(0, FrustumCullResult);

	RHIContext->Compute_Dispatch(8, 8, 1);

	RHIContext->Compute_TransitionResource(FrustumCullResult, State_Uav, State_GenerateRead);

	RHIContext->Compute_EndEvent();
#endif // ENABLE_COMPUTE_PIPELINE


	
}