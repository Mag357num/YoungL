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


	void CreateBasePassPSO_Static(IRHIContext* RHIContext, std::string PSOName);
	void CreateBasePassPSO_Skinned(IRHIContext* RHIContext, std::string PSOName);
	void CreateInstantcedPassPSO(IRHIContext* RHIContext, std::string PSOName);
	void CreateDepthPassPSO(IRHIContext* RHIContext, std::string PSOName);
	void CreatePresentPSO(IRHIContext* RHIContext, std::string PSOName);
	void CreatePostProcessPSOs(IRHIContext* RHIContext, FPostProcessing* PostProcessing);


	void CreateFrustumCullPSO(IRHIContext* RHIContext, std::string PSOName);




	IRHIGraphicsPipelineState* GetGraphicsPSO(std::string InPSOName);
	IRHIComputePipelineState* GeComputePSO(std::string InPSOName);

protected:
private:

	std::map<std::string, IRHIGraphicsPipelineState*> GraphicsPSOs;
	std::map<std::string, IRHIComputePipelineState*> ComputePSOs;
};