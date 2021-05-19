
#include "pch.h"
#include "RHIComputePipelineState_D3D12.h"

#include "RHIGraphicsPipelineState_D3D12.h"

#include "RHIResource_D3D12.h"
#include "RHIShaderResource_D3D12.h"

namespace D3D12RHI
{
	extern ComPtr<ID3D12Device> M_Device;
}

using namespace D3D12RHI;

namespace ShaderMap
{
	extern std::map<std::wstring, D3D12_SHADER_BYTECODE> GlobalCSShaderMap;
}
using namespace ShaderMap;


void FRHIComputePipelineState_D3D12::ParseShaderParameter(std::vector<CD3DX12_ROOT_PARAMETER>& InShaderParameters,
	std::vector<std::vector<CD3DX12_DESCRIPTOR_RANGE>>& D3D12Ranges)
{

	for (UINT Index = 0; Index < ShaderParameters.size(); ++Index)
	{
		CD3DX12_ROOT_PARAMETER Parameter;

		EShaderParaType Type = ShaderParameters[Index]->GetParameterType();
		switch (Type)
		{
		case ParaType_Range:
		{
			//size_t Start = D3D12Ranges.size();
			std::vector<CD3DX12_DESCRIPTOR_RANGE> TempD3D12Ranges;

			std::vector<FParameterRange>* Ranges = ShaderParameters[Index]->GetRangeTables();
			for (size_t RangeIndex = 0; RangeIndex < Ranges->size(); ++RangeIndex)
			{
				EParameterRangeType RangeType = Ranges->at(RangeIndex).RangeType;

				D3D12_DESCRIPTOR_RANGE_TYPE Type;
				switch (RangeType)
				{
				case RangeType_None:
					break;
				case RangeType_SRV:
					Type = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
					break;
				case RangeType_UAV:
					Type = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
					break;
				case RangeType_CBV:
					Type = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
					break;
				case RangeType_Sampler:
					Type = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
					break;
				default:
					break;
				}

				CD3DX12_DESCRIPTOR_RANGE Range;
				Range.Init(Type, Ranges->at(RangeIndex).NumParameters,
					Ranges->at(RangeIndex).ShaderRegister, Ranges->at(RangeIndex).ShaderRegisterSpace);
				TempD3D12Ranges.push_back(Range);

			}

			D3D12Ranges.push_back(TempD3D12Ranges);
			size_t TotalRangeSize = D3D12Ranges.size();
			Parameter.InitAsDescriptorTable((UINT)Ranges->size(), D3D12Ranges[TotalRangeSize - 1].data(),
				FRHIGraphicsPipelineState_D3D12::TranslateShaderVisibility(ShaderParameters[Index]->GetShaderVisibility()));
			InShaderParameters.push_back(Parameter);

		}
		break;

		case ParaType_Constant:
			Parameter.InitAsConstants(ShaderParameters[Index]->GetNum32BitValues(), ShaderParameters[Index]->GetShaderRegister(),
				ShaderParameters[Index]->GetShaderRegisterSpace(),
				FRHIGraphicsPipelineState_D3D12::TranslateShaderVisibility(ShaderParameters[Index]->GetShaderVisibility()));

			InShaderParameters.push_back(Parameter);
			break;

		case D3D12_DESCRIPTOR_RANGE_TYPE_CBV:
			Parameter.InitAsConstantBufferView(ShaderParameters[Index]->GetShaderRegister(), ShaderParameters[Index]->GetShaderRegisterSpace(),
				FRHIGraphicsPipelineState_D3D12::TranslateShaderVisibility(ShaderParameters[Index]->GetShaderVisibility()));

			InShaderParameters.push_back(Parameter);
			break;

		default:
			break;
		}

	}
}

void FRHIComputePipelineState_D3D12::CreateComputePSOInternal()
{

	//create root signature
	{
		std::vector<CD3DX12_ROOT_PARAMETER> DXShaderParameters;
		std::vector<std::vector<CD3DX12_DESCRIPTOR_RANGE>> D3D12Ranges;

		ParseShaderParameter(DXShaderParameters, D3D12Ranges);


		std::vector<CD3DX12_STATIC_SAMPLER_DESC> SamplerSates;
		ParseSamplerState(SamplerSates);

		// A root signature is an array of root parameters.
		CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc((UINT)DXShaderParameters.size(), DXShaderParameters.data(),
			(UINT)SamplerSates.size(), SamplerSates.data(),
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);


		// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
		ComPtr<ID3DBlob> serializedRootSig = nullptr;
		ComPtr<ID3DBlob> errorBlob = nullptr;
		HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
			serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

		if (errorBlob != nullptr)
		{
			::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
		}

		M_Device->CreateRootSignature(
			0,
			serializedRootSig->GetBufferPointer(),
			serializedRootSig->GetBufferSize(),
			IID_PPV_ARGS(&RootSignature));
	}

	//
	D3D12_COMPUTE_PIPELINE_STATE_DESC Desc;
	ZeroMemory(&Desc, sizeof(D3D12_COMPUTE_PIPELINE_STATE_DESC));

	Desc.pRootSignature = RootSignature.Get();
	Desc.CS = GlobalCSShaderMap[CS->GetShaderPath()];
	Desc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

	M_Device->CreateComputePipelineState(&Desc, IID_PPV_ARGS(&PSO));
}


void FRHIComputePipelineState_D3D12::ParseSamplerState(std::vector<CD3DX12_STATIC_SAMPLER_DESC>& InStaticSamplers)
{

	for (size_t SIndex = 0; SIndex < SamplerStates.size(); SIndex++)
	{
		CD3DX12_STATIC_SAMPLER_DESC Desc;
		Desc.ShaderRegister = SamplerStates[SIndex]->GetShaderRegister();
		Desc.RegisterSpace = SamplerStates[SIndex]->GetRegisterSpace();

		Desc.Filter = FRHIGraphicsPipelineState_D3D12::TranslateFileter(SamplerStates[SIndex]->GetSamplerFiler());
		Desc.AddressU = FRHIGraphicsPipelineState_D3D12::TranslateAddressMode(SamplerStates[SIndex]->GetAddressU());
		Desc.AddressV = FRHIGraphicsPipelineState_D3D12::TranslateAddressMode(SamplerStates[SIndex]->GetAddressV());
		Desc.AddressW = FRHIGraphicsPipelineState_D3D12::TranslateAddressMode(SamplerStates[SIndex]->GetAddressW());

		Desc.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		Desc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
		Desc.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL,
			Desc.MaxLOD = D3D12_FLOAT32_MAX;
		Desc.MinLOD = 0.f;
		Desc.MipLODBias = 0;
		Desc.MaxAnisotropy = 16;


		InStaticSamplers.push_back(Desc);
	}
}