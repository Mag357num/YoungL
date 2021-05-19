#pragma once

class IRHIComputePipelineState
{
public:
	IRHIComputePipelineState(){}
	virtual ~IRHIComputePipelineState(){

		if (SamplerStates.size() > 0)
		{
			for (int SIndex = 0; SIndex < SamplerStates.size(); SIndex++)
			{
				SamplerStates[SIndex].reset();
			}

			SamplerStates.clear();
			if (!SamplerStates.empty())
			{
				Utilities::Print("Empty SamplerStates Error! \n");
			}
		}

		if (ShaderParameters.size() > 0)
		{
			for (int PIndex = 0; PIndex < ShaderParameters.size(); PIndex++)
			{
				ShaderParameters[PIndex].reset();
			}

			ShaderParameters.clear();
			if (!ShaderParameters.empty())
			{
				Utilities::Print("Empty ShaderParameters Error! \n");
			}
		}


		if (CS)
		{
			CS.reset();
		}
	}

	void SetCS(IRHIShader* InShader);
	void AddSampleState(FRHISamplerState* InSampleState);
	void AddShaderParameter(FRHIShaderParameter* InShaderParameter);

	std::vector<std::unique_ptr<FRHISamplerState>>* GetSampleSates() { return &SamplerStates; }

	virtual void CreateComputePSOInternal() = 0;

protected:
	std::vector<std::unique_ptr<FRHISamplerState>> SamplerStates;
	std::vector<std::unique_ptr<FRHIShaderParameter>> ShaderParameters;

	std::unique_ptr<IRHIShader> CS;

private:
	
};
