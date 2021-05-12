#include "pch.h"
#include "RHIGraphicsPipelineState_D3D12.h"
//#include "RHIContext_D3D12.h"
#include "RHIResource_D3D12.h"
#include "RHIShaderResource_D3D12.h"

namespace D3D12RHI
{
	extern ComPtr<ID3D12Device> M_Device;
}

using namespace D3D12RHI;

namespace ShaderMap
{
	extern std::map<std::wstring, D3D12_SHADER_BYTECODE> GlobalShaderMap;
}
using namespace ShaderMap;


D3D12_SHADER_VISIBILITY TranslateShaderVisibility(EShaderParaVisibility Invisibility)
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


D3D12_FILTER TranslateFileter(ESamplerFilter InFilter)
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

D3D12_TEXTURE_ADDRESS_MODE TranslateAddressMode(ESampleAddress InAddress)
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

void FRHIGraphicsPipelineState_D3D12::ParseShaderParameter(std::vector<CD3DX12_ROOT_PARAMETER>& InShaderParameters, 
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
				TranslateShaderVisibility(ShaderParameters[Index]->GetShaderVisibility()));
			InShaderParameters.push_back(Parameter);

		}
		break;

		case ParaType_Constant:
			Parameter.InitAsConstants(ShaderParameters[Index]->GetNum32BitValues(), ShaderParameters[Index]->GetShaderRegister(),
				ShaderParameters[Index]->GetShaderRegisterSpace(),
			TranslateShaderVisibility(ShaderParameters[Index]->GetShaderVisibility()));

			InShaderParameters.push_back(Parameter);
			break;

		case D3D12_DESCRIPTOR_RANGE_TYPE_CBV:
			Parameter.InitAsConstantBufferView(ShaderParameters[Index]->GetShaderRegister(), ShaderParameters[Index]->GetShaderRegisterSpace(),
				TranslateShaderVisibility(ShaderParameters[Index]->GetShaderVisibility()));

			InShaderParameters.push_back(Parameter);
			break;

		default:
			break;
		}

	}
}

void FRHIGraphicsPipelineState_D3D12::CreateGraphicsPSOInternal()
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
	D3D12_GRAPHICS_PIPELINE_STATE_DESC Desc;
	ZeroMemory(&Desc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

	Desc.pRootSignature = RootSignature.Get();


	std::vector<D3D12_INPUT_ELEMENT_DESC> InputElementLayouts;
	ParseShaderInputElement(InputElementLayouts);
	Desc.InputLayout.NumElements = (UINT)InputElementLayouts.size();
	Desc.InputLayout.pInputElementDescs = InputElementLayouts.data();

	Desc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	Desc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	Desc.RasterizerState.CullMode = D3D12_CULL_MODE_FRONT;

	D3D12_DEPTH_STENCIL_DESC DSDesc;
	DSDesc.DepthEnable = DepthEnable;
	DSDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	DSDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	DSDesc.StencilEnable = FALSE;
	DSDesc.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
	DSDesc.StencilReadMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
	DSDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	DSDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	DSDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	DSDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;

	DSDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	DSDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	DSDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	DSDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;

	Desc.DepthStencilState = DSDesc;


	Desc.VS = GlobalShaderMap[VS->GetShaderPath()];
	Desc.PS = GlobalShaderMap[PS->GetShaderPath()];


	Desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	Desc.NumRenderTargets = 1;
	Desc.RTVFormats[0] = FRHIResource_D3D12::TranslateFormat(ColorTargetFormat);
	Desc.DSVFormat = FRHIResource_D3D12::TranslateFormat(DepthTargetFormat);
	Desc.SampleDesc.Count = 1;
	Desc.SampleDesc.Quality = 0;
	Desc.SampleMask = UINT_MAX;

	M_Device->CreateGraphicsPipelineState(&Desc, IID_PPV_ARGS(&PSO));
}


void FRHIGraphicsPipelineState_D3D12::ParseSamplerState(std::vector<CD3DX12_STATIC_SAMPLER_DESC>& InStaticSamplers)
{

	for (size_t SIndex = 0; SIndex < SamplerStates.size(); SIndex++)
	{
		CD3DX12_STATIC_SAMPLER_DESC Desc;
		Desc.ShaderRegister = SamplerStates[SIndex]->GetShaderRegister();
		Desc.RegisterSpace = SamplerStates[SIndex]->GetRegisterSpace();

		Desc.Filter = TranslateFileter(SamplerStates[SIndex]->GetSamplerFiler());
		Desc.AddressU = TranslateAddressMode(SamplerStates[SIndex]->GetAddressU());
		Desc.AddressV = TranslateAddressMode(SamplerStates[SIndex]->GetAddressV());
		Desc.AddressW = TranslateAddressMode(SamplerStates[SIndex]->GetAddressW());

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

void FRHIGraphicsPipelineState_D3D12::ParseShaderInputElement(std::vector<D3D12_INPUT_ELEMENT_DESC>& InElementLayouts)
{
	InElementLayouts.resize(ShaderInputElements.size());
	for (size_t Index = 0; Index < ShaderInputElements.size(); ++Index)
	{
		//std::string SemanticName  = ShaderInputElements[Index]->GetSemanticName();
		InElementLayouts[Index].SemanticName = ShaderInputElements[Index]->SemanticName.c_str();
		InElementLayouts[Index].SemanticIndex = ShaderInputElements[Index]->GetSemanticIndex();
		InElementLayouts[Index].Format = FRHIResource_D3D12::TranslateFormat(ShaderInputElements[Index]->GetFormat());
		InElementLayouts[Index].InputSlot = ShaderInputElements[Index]->GetInputSlot();
		InElementLayouts[Index].AlignedByteOffset = ShaderInputElements[Index]->GetAlignedByteOffset();
		InElementLayouts[Index].InputSlotClass = (D3D12_INPUT_CLASSIFICATION)ShaderInputElements[Index]->GetInputSlotClass();
		InElementLayouts[Index].InstanceDataStepRate = ShaderInputElements[Index]->GetInstanceDataStepRate();

	}
}