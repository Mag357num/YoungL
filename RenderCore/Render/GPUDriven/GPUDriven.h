#pragma once
#include "../RHI/RHIContext.h"
#include "../PSOManager.h"
#include "GPUDrivenProcess.h"

class FGPUDriven
{
public:
	FGPUDriven();
	~FGPUDriven();

	void PopulateComputePipeline(IRHIContext* RHIContext, FPSOManager* InPSOManager);

	void InitFrustumCull(IRHIContext* RHIContext, FPSOManager* InPSOManager);

private:

	std::vector<FGPUDrivenProcess*> Processes;
};
