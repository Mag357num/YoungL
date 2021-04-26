#pragma once
#include "RHISamplerState.h"
#include "RHIShaderParameter.h"

//#include <vector>
//#include <memory>

class IRHIGraphicsPipelineState
{
public:
	IRHIGraphicsPipelineState() {}
	virtual ~IRHIGraphicsPipelineState() {
	
		for (int SIndex = 0; SIndex < SampleStates.size(); SIndex++)
		{
			SampleStates[SIndex].reset();
		}

		if (!SampleStates.empty())
		{
		}

		for (int PIndex = 0; PIndex < ShaderParameters.size(); PIndex++)
		{
			ShaderParameters[PIndex].reset();
		}

		if (!ShaderParameters.empty())
		{
		}
	
	}
protected:

	std::vector<std::unique_ptr<FRHISamplerState>> SampleStates;
	std::vector<std::unique_ptr<FRHIShaderParameter>> ShaderParameters;
private:
};