#include "pch.h"
#include "RHIGraphicsPipelineState_D3D12.h"
//#include "RHIContext_D3D12.h"
#include "RHIResource_D3D12.h"


namespace D3D12RHI
{
	extern ComPtr<ID3D12Device> M_Device;
}

using namespace D3D12RHI;

void FRHIGraphicsPipelineState_D3D12::CreateGraphicsPSOInternal()
{

	//create root signature
	{
		std::vector<CD3DX12_ROOT_PARAMETER> ShaderParameters;
		ParseShaderParameter(ShaderParameters);

		std::vector<CD3DX12_STATIC_SAMPLER_DESC> SamplerSates;
		ParseSamplerState(SamplerSates);

		// A root signature is an array of root parameters.
		CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc((UINT)ShaderParameters.size(), ShaderParameters.data(), 
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
	//draw rect dont't need input layout
	Desc.InputLayout.NumElements = 0;
	Desc.InputLayout.pInputElementDescs = nullptr;

	Desc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	Desc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	Desc.RasterizerState.CullMode = D3D12_CULL_MODE_FRONT;

	D3D12_DEPTH_STENCIL_DESC DSDesc;
	DSDesc.DepthEnable = TRUE;
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

	//Desc.VS =
	//{
	//	g_ScreenVS,
	//	sizeof(g_ScreenVS)
	//};
	//Desc.PS =
	//{
	//	g_ScreenPS,
	//	sizeof(g_ScreenPS)
	//};

	Desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	Desc.NumRenderTargets = 1;
	Desc.RTVFormats[0] = FRHIResource_D3D12::TranslateFormat(ColorTargetFormat);
	Desc.DSVFormat = FRHIResource_D3D12::TranslateFormat(DepthTargetFormat);
	Desc.SampleDesc.Count = 1;
	Desc.SampleDesc.Quality = 0;
	Desc.SampleMask = UINT_MAX;

	M_Device->CreateGraphicsPipelineState(&Desc, IID_PPV_ARGS(&PSO));
}

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

void FRHIGraphicsPipelineState_D3D12::ParseShaderParameter(std::vector<CD3DX12_ROOT_PARAMETER>& InShaderParameters)
{
		//CD3DX12_ROOT_PARAMETER slotRootParameter[1];

		// Create a single descriptor table of CBVs.
		//CD3DX12_DESCRIPTOR_RANGE cbvTable;
		//cbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);
		//slotRootParameter[0].InitAsDescriptorTable(1, &cbvTable, D3D12_SHADER_VISIBILITY_PIXEL);
	for (UINT Index = 0; Index < ShaderParameters.size(); ++Index)
	{
		CD3DX12_ROOT_PARAMETER Parameter;

		EShaderParaType Type = ShaderParameters[Index]->GetParameterType();
		switch (Type)
		{
			case ParaType_Range:
				{
					std::vector<CD3DX12_DESCRIPTOR_RANGE> D3D12Ranges;

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
						D3D12Ranges.push_back(Range);
					}

					Parameter.InitAsDescriptorTable((UINT)D3D12Ranges.size(), D3D12Ranges.data(),
						TranslateShaderVisibility(ShaderParameters[Index]->GetShaderVisibility()));
				}
				break;

			default:
				break;
		}

		InShaderParameters.push_back(Parameter);
	}
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

void FRHIGraphicsPipelineState_D3D12::ParseSamplerState(std::vector<CD3DX12_STATIC_SAMPLER_DESC>& InStaticSamplers)
{
	//const CD3DX12_STATIC_SAMPLER_DESC anisotropicWrap(
	//	4, // shaderRegister
	//	D3D12_FILTER_ANISOTROPIC, // filter
	//	D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
	//	D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
	//	D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressW
	//	0.0f,                             // mipLODBias
	//	8);

	for (size_t SIndex = 0; SIndex < SamplerStates.size(); SIndex++)
	{
		CD3DX12_STATIC_SAMPLER_DESC Desc;
		Desc.ShaderRegister = SamplerStates[SIndex]->GetShaderRegister();
		Desc.RegisterSpace = SamplerStates[SIndex]->GetRegisterSpace();

		Desc.Filter = TranslateFileter(SamplerStates[SIndex]->GetSamplerFiler());
		Desc.AddressU = TranslateAddressMode(SamplerStates[SIndex]->GetAddressU());
		Desc.AddressV = TranslateAddressMode(SamplerStates[SIndex]->GetAddressV());
		Desc.AddressW = TranslateAddressMode(SamplerStates[SIndex]->GetAddressW());

		InStaticSamplers.push_back(Desc);
	}
}