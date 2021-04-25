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

		RootSignature.Reset();

		//for (int Index = 0; Index < Descriptors.size(); Index++)
		//{
		//	Descriptors[Index].Reset();
		//}
		//Descriptors.empty();
	}


	ComPtr<ID3D12PipelineState>	PSO;

	//for shader binding 
	//todo: shader binding for RHI
	ComPtr<ID3D12RootSignature> RootSignature;
	//std::vector<ComPtr<ID3D12DescriptorHeap>> Descriptors;
private:
	
};

