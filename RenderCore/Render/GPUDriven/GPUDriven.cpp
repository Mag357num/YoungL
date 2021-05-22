#include "pch.h"
#include "GPUDriven.h"
#include "FrustumCull.h"

FGPUDriven::FGPUDriven()
{

}

FGPUDriven::~FGPUDriven()
{

	if (Processes.size() > 0)
	{
		for (size_t Index = 0; Index < Processes.size(); ++Index)
		{
			delete Processes[Index];
		}
		Processes.clear();
	}
}

void FGPUDriven::PopulateComputePipeline(IRHIContext* RHIContext, FPSOManager* InPSOManager)
{
#ifdef ENABLE_COMPUTE_PIPELINE
	RHIContext->Compute_BeginDraw(L"GPUDriven");
	RHIContext->Compute_PrepareShaderParameter();
	for (size_t Index = 0; Index < Processes.size(); ++Index)
	{
		if (Processes[Index]->GetActive())
		{
			Processes[Index]->Run(RHIContext, InPSOManager);
		}
	}
	RHIContext->Compute_EndDraw();

	RHIContext->WaitForComputeTask();
#endif // ENABLE_COMPUTE_PIPELINE


}

void FGPUDriven::InitFrustumCull(IRHIContext* RHIContext, FPSOManager* InPSOManager)
{
	FFrustumCull* FrustumCull= new FFrustumCull();
	FrustumCull->InitialFrustumCull(RHIContext, InPSOManager);
	FrustumCull->SetActive(true);

	Processes.push_back(FrustumCull);

}