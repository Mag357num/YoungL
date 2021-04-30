#pragma once
#include <wrl.h>
#include "d3dx12.h"

using namespace Microsoft::WRL;

#include "../RHIGraphicsPipelineState.h"

class FRHIGraphicsPipelineState_D3D12 : public IRHIGraphicsPipelineState
{
public:
	FRHIGraphicsPipelineState_D3D12(){
		ShadersInputDesc =
		{
			 { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			 { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			 { "TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};
	}
	virtual ~FRHIGraphicsPipelineState_D3D12() {
		PSO.Reset();

		RootSignature.Reset();

		//for (int Index = 0; Index < Descriptors.size(); Index++)
		//{
		//	Descriptors[Index].Reset();
		//}
		//Descriptors.empty();

		if (!ShadersInputDesc.empty())
		{
		}
	}


	virtual void CreateGraphicsPSOInternal()override;

	ComPtr<ID3D12PipelineState>	PSO;

	//for shader binding 
	//todo: shader binding for RHI
	ComPtr<ID3D12RootSignature> RootSignature;
	//std::vector<ComPtr<ID3D12DescriptorHeap>> Descriptors;
private:
	void ParseShaderParameter(std::vector<CD3DX12_ROOT_PARAMETER>& InShaderParameters);
	void ParseSamplerState(std::vector<CD3DX12_STATIC_SAMPLER_DESC>& InStaticSamplers);

	std::vector<D3D12_INPUT_ELEMENT_DESC> ShadersInputDesc;
};

