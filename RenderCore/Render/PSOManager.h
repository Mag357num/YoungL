#pragma once
#include "RHI/RHIGraphicsPipelineState.h"
#include "RHI/RHIComputePipelineState.h"
#include "RHI/RHIContext.h"

#include "PostProcessing.h"

#include <map>

class FPSOManager
{
public:
	FPSOManager();
	~FPSOManager();


	void CreateBasePassPSO_Static(IRHIContext* RHIContext);
	void CreateBasePassPSO_Skinned(IRHIContext* RHIContext);
	void CreateInstantcedPassPSO(IRHIContext* RHIContext);
	void CreateDepthPassPSO(IRHIContext* RHIContext);
	void CreatePresentPSO(IRHIContext* RHIContext);
	void CreatePostProcessPSOs(IRHIContext* RHIContext, FPostProcessing* PostProcessing);


	IRHIGraphicsPipelineState* GetGraphicsPSO(std::string InPSOName);
	IRHIComputePipelineState* GeComputePSO(std::string InPSOName);

protected:
private:

	std::map<std::string, IRHIGraphicsPipelineState*> GraphicsPSOs;
	std::map<std::string, IRHIComputePipelineState*> ComputePSOs;
};