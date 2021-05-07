#pragma once
#include "RHISamplerState.h"
#include "RHIShaderParameter.h"
#include "RHIShader.h"
//#include <vector>
//#include <memory>

class IRHIGraphicsPipelineState
{
public:
	IRHIGraphicsPipelineState() {}
	virtual ~IRHIGraphicsPipelineState() {
	
		for (int SIndex = 0; SIndex < SamplerStates.size(); SIndex++)
		{
			SamplerStates[SIndex].reset();
		}

		if (!SamplerStates.empty())
		{
		}

		for (int PIndex = 0; PIndex < ShaderParameters.size(); PIndex++)
		{
			ShaderParameters[PIndex].reset();
		}

		if (!ShaderParameters.empty())
		{
		}

		if (VS)
		{
			VS.reset();
		}

		if (PS)
		{
			PS.reset();
		}
	
	}

	void SetVS(IRHIShader* InShader);
	void SetPS(IRHIShader* InShader);

	void SetCorlorTargetFormat(EPixelBufferFormat InFormat);
	void SetDepthTargetFormat(EPixelBufferFormat InFormat);

	void AddSampleState(FRHISamplerState* InSampleState);
	void AddShaderParameter(FRHIShaderParameter* InShaderParameter);

	std::vector<std::unique_ptr<FRHISamplerState>>* GetSampleSates(){return &SamplerStates;}

	virtual void CreateGraphicsPSOInternal() = 0;

protected:

	std::vector<std::unique_ptr<FRHISamplerState>> SamplerStates;
	std::vector<std::unique_ptr<FRHIShaderParameter>> ShaderParameters;

	EPixelBufferFormat ColorTargetFormat;
	EPixelBufferFormat DepthTargetFormat;

	std::unique_ptr<IRHIShader> VS;
	std::unique_ptr<IRHIShader> PS;

private:

};