#include "pch.h"
#include "RHIComputePipelineState.h"

void IRHIComputePipelineState::AddSampleState(FRHISamplerState* InSampleState)
{
	SamplerStates.push_back(std::make_unique<FRHISamplerState>(*InSampleState));
}

void IRHIComputePipelineState::AddShaderParameter(FRHIShaderParameter* InShaderParameter)
{
	ShaderParameters.push_back(std::make_unique<FRHIShaderParameter>(*InShaderParameter));
}

void IRHIComputePipelineState::SetCS(IRHIShader* InShader)
{
	CS = std::make_unique<IRHIShader>(*InShader);
}
