#pragma once
#include "../RHI/RHIContext.h"
#include "../PSOManager.h"

class FGPUDriven
{
public:
	FGPUDriven();
	~FGPUDriven();

	void InitFrustumCull(IRHIContext* RHIContext, FPSOManager* InPSOManager);

private:
	FRHIColorResource*  FrustumCullResult;
};
