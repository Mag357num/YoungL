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

void FGPUDriven::PopulateGPUDriven(IRHIContext* RHIContext, FPSOManager* InPSOManager)
{
	RHIContext->BeginEvent(L"GPUDriven");
	for (size_t Index = 0; Index < Processes.size(); ++Index)
	{
		if (Processes[Index]->GetActive())
		{
			Processes[Index]->Run(RHIContext, InPSOManager);
		}
	}
	RHIContext->EndEvent();
}

void FGPUDriven::InitFrustumCull(IRHIContext* RHIContext, FPSOManager* InPSOManager)
{
	FFrustumCull* FrustumCull= new FFrustumCull();
	FrustumCull->InitialFrustumCull(RHIContext, InPSOManager);
	FrustumCull->SetActive(true);

	Processes.push_back(FrustumCull);

}