#include "pch.h"
#include "RHIGraphicsPipelineState.h"

void IRHIGraphicsPipelineState::AddSampleState(FRHISamplerState* InSampleState)
{
	SamplerStates.push_back(std::make_unique<FRHISamplerState>(*InSampleState));
}

void IRHIGraphicsPipelineState::AddShaderParameter(FRHIShaderParameter* InShaderParameter)
{
	ShaderParameters.push_back(std::make_unique<FRHIShaderParameter>(*InShaderParameter));
}

void IRHIGraphicsPipelineState::SetCorlorTargetFormat(EPixelBufferFormat InFormat)
{
	ColorTargetFormat = InFormat;
}

void IRHIGraphicsPipelineState::SetDepthTargetFormat(EPixelBufferFormat InFormat)
{
	DepthTargetFormat = InFormat;
}

void IRHIGraphicsPipelineState::SetVS(IRHIShader* InShader)
{
	VS = std::make_unique<IRHIShader>(*InShader);
}

void IRHIGraphicsPipelineState::SetPS(IRHIShader* InShader)
{
	PS = std::make_unique<IRHIShader>(*InShader);
}