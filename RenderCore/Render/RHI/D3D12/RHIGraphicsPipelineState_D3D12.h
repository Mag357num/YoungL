#pragma once
#include <wrl.h>
#include "d3dx12.h"

using namespace Microsoft::WRL;

#include "../RHIGraphicsPipelineState.h"

class FRHIGraphicsPipelineState_D3D12 : public IRHIGraphicsPipelineState
{
public:
	FRHIGraphicsPipelineState_D3D12(){

	}
	virtual ~FRHIGraphicsPipelineState_D3D12() {
		PSO.Reset();

		RootSignature.Reset();

	}


	static D3D12_SHADER_VISIBILITY TranslateShaderVisibility(EShaderParaVisibility Invisibility)
	{
		D3D12_SHADER_VISIBILITY Visibility = D3D12_SHADER_VISIBILITY_ALL;
		switch (Invisibility)
		{
		case Visibility_None:
			break;
		case Visibility_VS:
			Visibility = D3D12_SHADER_VISIBILITY_VERTEX;
			break;
		case Visibility_PS:
			Visibility = D3D12_SHADER_VISIBILITY_PIXEL;
			break;
		case Visibility_All:
			Visibility = D3D12_SHADER_VISIBILITY_ALL;
			break;
		default:
			break;
		}

		return Visibility;
	}


	static D3D12_FILTER TranslateFileter(ESamplerFilter InFilter)
	{
		D3D12_FILTER OutFilter = D3D12_FILTER_MIN_MAG_MIP_POINT;
		switch (InFilter)
		{
		case Filter_MIN_MAG_LINEAR_MIP_POINT:
			OutFilter = D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
			break;
		default:
			break;
		}

		return OutFilter;
	}

	static D3D12_TEXTURE_ADDRESS_MODE TranslateAddressMode(ESampleAddress InAddress)
	{
		D3D12_TEXTURE_ADDRESS_MODE Mode;

		switch (InAddress)
		{
		case Address_None:
			break;
		case ADDRESS_MODE_WRAP:
			Mode = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
			break;
		case ADDRESS_MODE_MIRROR:
			Mode = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
			break;
		case ADDRESS_MODE_CLAMP:
			Mode = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
			break;
		case ADDRESS_MODE_BORDER:
			Mode = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
			break;
		case ADDRESS_MODE_MIRROR_ONCE:
			Mode = D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE;
			break;
		default:
			break;
		}

		return Mode;
	}


	virtual void CreateGraphicsPSOInternal()override;

	ComPtr<ID3D12PipelineState>	PSO;

	//for shader binding 
	//todo: shader binding for RHI
	ComPtr<ID3D12RootSignature> RootSignature;
private:
	void ParseShaderParameter(std::vector<CD3DX12_ROOT_PARAMETER>& InShaderParameters, std::vector<std::vector<CD3DX12_DESCRIPTOR_RANGE>>& D3D12Ranges);
	void ParseSamplerState(std::vector<CD3DX12_STATIC_SAMPLER_DESC>& InStaticSamplers);
	void ParseShaderInputElement(std::vector<D3D12_INPUT_ELEMENT_DESC>& InElementLayouts);

};

