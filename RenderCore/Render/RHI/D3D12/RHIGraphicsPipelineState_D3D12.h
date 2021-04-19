#pragma once
#include <wrl.h>

using namespace Microsoft::WRL;

#include "../RHIContext.h"

class FRHIGraphicsPipelineState_D3D12 : public IRHIGraphicsPipelineState
{
public:
	FRHIGraphicsPipelineState_D3D12(){}
	virtual ~FRHIGraphicsPipelineState_D3D12() {
		PSO.Reset();
	}

	ComPtr<ID3D12PipelineState>	PSO;
private:
	
};

