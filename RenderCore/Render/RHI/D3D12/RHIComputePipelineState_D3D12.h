#pragma once

#include "../RHIComputePipelineState.h"

#include <wrl.h>
#include "d3dx12.h"

using namespace Microsoft::WRL;

class FRHIComputePipelineState_D3D12 : public IRHIComputePipelineState
{
public:
	FRHIComputePipelineState_D3D12(){}
	virtual ~FRHIComputePipelineState_D3D12()
	{
		PSO.Reset();

		RootSignature.Reset();
	}

	virtual void CreateComputePSOInternal()override;

	ComPtr<ID3D12PipelineState>	PSO;

	//for shader binding 
	//todo: shader binding for RHI
	ComPtr<ID3D12RootSignature> RootSignature;
private:
	void ParseShaderParameter(std::vector<CD3DX12_ROOT_PARAMETER>& InShaderParameters, std::vector<std::vector<CD3DX12_DESCRIPTOR_RANGE>>& D3D12Ranges);
	void ParseSamplerState(std::vector<CD3DX12_STATIC_SAMPLER_DESC>& InStaticSamplers);
};