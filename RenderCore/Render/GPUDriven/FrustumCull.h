#pragma once

#include "GPUDrivenProcess.h"
#include "../RHI/RHIContext.h"
#include "../PSOManager.h"


class FFrustumCull : public FGPUDrivenProcess
{
public:
	FFrustumCull(){};
	virtual  ~FFrustumCull(){
		if (FrustumCullResult)
		{
			delete FrustumCullResult;
			FrustumCullResult = nullptr;
		}
	};

	void InitialFrustumCull(IRHIContext* RHIContext, FPSOManager* InPSOManager);

	virtual void Run(IRHIContext* RHIContext, FPSOManager* InPSOManager)override;

private:
	FRHIColorResource* FrustumCullResult;
};
