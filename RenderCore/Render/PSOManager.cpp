#include "pch.h"
#include "PSOManager.h"

FPSOManager::FPSOManager()
{

}

FPSOManager::~FPSOManager()
{
	//release pso state object
	if (GraphicsPSOs.size() > 0)
	{
		for (auto It = GraphicsPSOs.begin(); It != GraphicsPSOs.end(); ++It)
		{
			delete It->second;
			It->second = nullptr;
		}
		//empty
		if (!GraphicsPSOs.empty())
		{
			Utilities::Print("Empty GraphicsPSOs Error! \n");
		}
	}


	if (ComputePSOs.size() > 0)
	{
		for (auto It = ComputePSOs.begin(); It != ComputePSOs.end(); ++It)
		{
			delete It->second;
			It->second = nullptr;
		}

		//empty
		if (!ComputePSOs.empty())
		{
			Utilities::Print("Empty ComputePSOs Error! \n");
		}
	}

}


void FPSOManager::CreateBasePassPSO_Static(IRHIContext* RHIContext)
{
	IRHIGraphicsPipelineState* BasePassPSO_Static = RHIContext->CreateEmpltyGraphicsPSO();

	//for shader parameter
	{
		FRHIShaderParameter SlotPara0(ParaType_CBV, 0, 0, Visibility_All);
		BasePassPSO_Static->AddShaderParameter(&SlotPara0);

		FRHIShaderParameter SlotPara1(ParaType_CBV, 1, 0, Visibility_All);
		BasePassPSO_Static->AddShaderParameter(&SlotPara1);

		FParameterRange ParamRange(RangeType_SRV, 1, 0, 0);
		FRHIShaderParameter SlotPara2(ParaType_Range, 0, 0, Visibility_PS);
		SlotPara2.AddRangeTable(ParamRange);
		BasePassPSO_Static->AddShaderParameter(&SlotPara2);

	}

	{
		FRHIShaderInputElement InputElement0("POSITION", 0, PixelFormat_R32G32B32_Float, 0, 0, INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0);
		BasePassPSO_Static->AddShaderInputElement(&InputElement0);

		FRHIShaderInputElement InputElement1("NORMAL", 0, PixelFormat_R32G32B32_Float, 0, 12, INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0);
		BasePassPSO_Static->AddShaderInputElement(&InputElement1);

		FRHIShaderInputElement InputElement2("TEXCOORD", 0, PixelFormat_R32G32_Float, 0, 24, INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0);
		BasePassPSO_Static->AddShaderInputElement(&InputElement2);
	}


	BasePassPSO_Static->SetCorlorTargetFormat(PixelFormat_R16G16B16A16_Float);
	BasePassPSO_Static->SetDepthTargetFormat(PixelFormat_D24_UNORM_S8_UINT);

	IRHIShader* VS = new IRHIShader();
	VS->SetShaderType(ShaderType_VS);
	VS->SetShaderPath(L"BasePassVS");
	BasePassPSO_Static->SetVS(VS);

	IRHIShader* PS = new IRHIShader();
	PS->SetShaderType(ShaderType_PS);
	PS->SetShaderPath(L"BasePassPS");
	BasePassPSO_Static->SetPS(PS);

	BasePassPSO_Static->SetDepthEnable(TRUE);

	BasePassPSO_Static->CreateGraphicsPSOInternal();
	GraphicsPSOs.insert(std::make_pair("BasePass", BasePassPSO_Static));
}

void FPSOManager::CreateBasePassPSO_Skinned(IRHIContext* RHIContext)
{
	IRHIGraphicsPipelineState* BasePassPSO_Skinned = RHIContext->CreateEmpltyGraphicsPSO();

	//for shader parameter
	{
		FRHIShaderParameter SlotPara0(ParaType_CBV, 0, 0, Visibility_All);
		BasePassPSO_Skinned->AddShaderParameter(&SlotPara0);

		FRHIShaderParameter SlotPara1(ParaType_CBV, 1, 0, Visibility_All);
		BasePassPSO_Skinned->AddShaderParameter(&SlotPara1);

		FParameterRange ParamRange(RangeType_SRV, 1, 0, 0);
		FRHIShaderParameter SlotPara2(ParaType_Range, 0, 0, Visibility_PS);
		SlotPara2.AddRangeTable(ParamRange);
		BasePassPSO_Skinned->AddShaderParameter(&SlotPara2);

		FRHIShaderParameter SlotPara3(ParaType_CBV, 2, 0, Visibility_All);
		BasePassPSO_Skinned->AddShaderParameter(&SlotPara3);
	}

	{
		FRHIShaderInputElement InputElement0("POSITION", 0, PixelFormat_R32G32B32_Float, 0, 0, INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0);
		BasePassPSO_Skinned->AddShaderInputElement(&InputElement0);

		FRHIShaderInputElement InputElement1("NORMAL", 0, PixelFormat_R32G32B32_Float, 0, 12, INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0);
		BasePassPSO_Skinned->AddShaderInputElement(&InputElement1);

		FRHIShaderInputElement InputElement2("TEXCOORD", 0, PixelFormat_R32G32_Float, 0, 24, INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0);
		BasePassPSO_Skinned->AddShaderInputElement(&InputElement2);

		FRHIShaderInputElement InputElement3("TAGANT", 0, PixelFormat_R32G32B32_Float, 0, 32, INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0);
		BasePassPSO_Skinned->AddShaderInputElement(&InputElement3);

		FRHIShaderInputElement InputElement4("BONEINDEX", 0, PixelFormat_R32G32B32A32_UINT, 0, 44, INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0);
		BasePassPSO_Skinned->AddShaderInputElement(&InputElement4);

		FRHIShaderInputElement InputElement5("BONEWEIGHT", 0, PixelFormat_R32G32B32A32_Float, 0, 60, INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0);
		BasePassPSO_Skinned->AddShaderInputElement(&InputElement5);
	}


	BasePassPSO_Skinned->SetCorlorTargetFormat(PixelFormat_R16G16B16A16_Float);
	BasePassPSO_Skinned->SetDepthTargetFormat(PixelFormat_D24_UNORM_S8_UINT);

	IRHIShader* VS = new IRHIShader();
	VS->SetShaderType(ShaderType_VS);
	VS->SetShaderPath(L"SkinnedMeshVS");
	BasePassPSO_Skinned->SetVS(VS);

	IRHIShader* PS = new IRHIShader();
	PS->SetShaderType(ShaderType_PS);
	PS->SetShaderPath(L"SkinnedMeshPS");
	BasePassPSO_Skinned->SetPS(PS);

	BasePassPSO_Skinned->SetDepthEnable(TRUE);

	BasePassPSO_Skinned->CreateGraphicsPSOInternal();
	GraphicsPSOs.insert(std::make_pair("SkinPass", BasePassPSO_Skinned));
}

void FPSOManager::CreateInstantcedPassPSO(IRHIContext* RHIContext)
{
	IRHIGraphicsPipelineState* InstancePassPSO = RHIContext->CreateEmpltyGraphicsPSO();

	//for shader parameter
	{
		FRHIShaderParameter SlotPara0(ParaType_CBV, 0, 0, Visibility_All);
		InstancePassPSO->AddShaderParameter(&SlotPara0);

		FRHIShaderParameter SlotPara1(ParaType_CBV, 1, 0, Visibility_All);
		InstancePassPSO->AddShaderParameter(&SlotPara1);

		FParameterRange ParamRange(RangeType_SRV, 1, 0, 0);
		FRHIShaderParameter SlotPara2(ParaType_Range, 0, 0, Visibility_PS);
		SlotPara2.AddRangeTable(ParamRange);
		InstancePassPSO->AddShaderParameter(&SlotPara2);

		FRHIShaderParameter SlotPara3(ParaType_Constant, 2, 0, Visibility_All);
		SlotPara3.SetNum32BitValues(2);
		InstancePassPSO->AddShaderParameter(&SlotPara3);

	}

	{
		FRHIShaderInputElement InputElement0("POSITION", 0, PixelFormat_R32G32B32_Float, 0, 0, INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0);
		InstancePassPSO->AddShaderInputElement(&InputElement0);

		FRHIShaderInputElement InputElement1("NORMAL", 0, PixelFormat_R32G32B32_Float, 0, 12, INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0);
		InstancePassPSO->AddShaderInputElement(&InputElement1);

		FRHIShaderInputElement InputElement2("TEXCOORD", 0, PixelFormat_R32G32_Float, 0, 24, INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0);
		InstancePassPSO->AddShaderInputElement(&InputElement2);

		FRHIShaderInputElement InputElement3("INS_TRANS0ROW", 0, PixelFormat_R32G32B32A32_Float, 1, 0, INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1);
		InstancePassPSO->AddShaderInputElement(&InputElement3);

		FRHIShaderInputElement InputElement4("INS_TRANS1ROW", 0, PixelFormat_R32G32B32A32_Float, 1, 16, INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1);
		InstancePassPSO->AddShaderInputElement(&InputElement4);

		FRHIShaderInputElement InputElement5("INS_TRANS2ROW", 0, PixelFormat_R32G32B32A32_Float, 1, 32, INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1);
		InstancePassPSO->AddShaderInputElement(&InputElement5);

		FRHIShaderInputElement InputElement6("INS_TRANS3ROW", 0, PixelFormat_R32G32B32A32_Float, 1, 48, INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1);
		InstancePassPSO->AddShaderInputElement(&InputElement6);
	}


	InstancePassPSO->SetCorlorTargetFormat(PixelFormat_R16G16B16A16_Float);
	InstancePassPSO->SetDepthTargetFormat(PixelFormat_D24_UNORM_S8_UINT);

	IRHIShader* VS = new IRHIShader();
	VS->SetShaderType(ShaderType_VS);
	VS->SetShaderPath(L"InstancePassVS");
	InstancePassPSO->SetVS(VS);

	IRHIShader* PS = new IRHIShader();
	PS->SetShaderType(ShaderType_PS);
	PS->SetShaderPath(L"BasePassPS");
	InstancePassPSO->SetPS(PS);

	InstancePassPSO->SetDepthEnable(TRUE);

	InstancePassPSO->CreateGraphicsPSOInternal();
	GraphicsPSOs.insert(std::make_pair("InstancePass", InstancePassPSO));
}

void FPSOManager::CreateDepthPassPSO(IRHIContext* RHIContext)
{
	IRHIGraphicsPipelineState* DepthPassPSO = RHIContext->CreateEmpltyGraphicsPSO();

	//for shader parameter
	{
		FRHIShaderParameter SlotPara0(ParaType_CBV, 0, 0, Visibility_All);
		DepthPassPSO->AddShaderParameter(&SlotPara0);

		FRHIShaderParameter SlotPara1(ParaType_CBV, 1, 0, Visibility_All);
		DepthPassPSO->AddShaderParameter(&SlotPara1);

	}

	{
		FRHIShaderInputElement InputElement0("POSITION", 0, PixelFormat_R32G32B32_Float, 0, 0, INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0);
		DepthPassPSO->AddShaderInputElement(&InputElement0);

		FRHIShaderInputElement InputElement1("NORMAL", 0, PixelFormat_R32G32B32_Float, 0, 12, INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0);
		DepthPassPSO->AddShaderInputElement(&InputElement1);

		FRHIShaderInputElement InputElement2("TEXCOORD", 0, PixelFormat_R32G32_Float, 0, 24, INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0);
		DepthPassPSO->AddShaderInputElement(&InputElement2);
	}


	DepthPassPSO->SetCorlorTargetFormat(PixelFormat_R16G16B16A16_Float);
	DepthPassPSO->SetDepthTargetFormat(PixelFormat_D24_UNORM_S8_UINT);

	IRHIShader* VS = new IRHIShader();
	VS->SetShaderType(ShaderType_VS);
	VS->SetShaderPath(L"DepthVS");
	DepthPassPSO->SetVS(VS);

	IRHIShader* PS = new IRHIShader();
	PS->SetShaderType(ShaderType_PS);
	PS->SetShaderPath(L"DepthPS");
	DepthPassPSO->SetPS(PS);

	DepthPassPSO->SetDepthEnable(TRUE);

	DepthPassPSO->CreateGraphicsPSOInternal();
	GraphicsPSOs.insert(std::make_pair("DepthPass", DepthPassPSO));
}

void FPSOManager::CreatePresentPSO(IRHIContext* RHIContext)
{
	//create present pass
	IRHIGraphicsPipelineState* PresentPSO = RHIContext->CreateEmpltyGraphicsPSO();

	FParameterRange ParamRange(RangeType_SRV, 1, 0, 0);
	FRHIShaderParameter ShaderParam(ParaType_Range, 0, 0, Visibility_PS);
	ShaderParam.AddRangeTable(ParamRange);
	PresentPSO->AddShaderParameter(&ShaderParam);

	FRHIShaderParameter ConstantPara(ParaType_Constant, 0, 0, Visibility_PS);
	ConstantPara.SetNum32BitValues(2);
	PresentPSO->AddShaderParameter(&ConstantPara);

	PresentPSO->SetCorlorTargetFormat(PixelFormat_R8G8B8A8_Unorm);

	FRHISamplerState SampleState(0, 0, Filter_MIN_MAG_LINEAR_MIP_POINT, ADDRESS_MODE_CLAMP, ADDRESS_MODE_CLAMP, ADDRESS_MODE_CLAMP);
	PresentPSO->AddSampleState(&SampleState);

	IRHIShader* VS = new IRHIShader();
	VS->SetShaderType(ShaderType_VS);
	VS->SetShaderPath(L"ScreenVS");
	PresentPSO->SetVS(VS);

	IRHIShader* PS = new IRHIShader();
	PS->SetShaderType(ShaderType_PS);
	PS->SetShaderPath(L"ScreenPS");
	PresentPSO->SetPS(PS);

	PresentPSO->CreateGraphicsPSOInternal();

	GraphicsPSOs.insert(std::make_pair("PresentPass", PresentPSO));
}

void FPSOManager::CreatePostProcessPSOs(IRHIContext* RHIContext, FPostProcessing* PostProcessing)
{
	IRHIGraphicsPipelineState* BloomSetUpPSO = PostProcessing->CreateBloomSetUpPSO(RHIContext);
	GraphicsPSOs.insert(std::make_pair("BloomSetUp", BloomSetUpPSO));

	IRHIGraphicsPipelineState* BloomDownPSO = PostProcessing->CreateBloomDownPSO(RHIContext);
	GraphicsPSOs.insert(std::make_pair("BloomDown", BloomDownPSO));

	IRHIGraphicsPipelineState* BloomUpPSO = PostProcessing->CreateBloomUpPSO(RHIContext);
	GraphicsPSOs.insert(std::make_pair("BloomUp", BloomUpPSO));

	IRHIGraphicsPipelineState* BloomSunMergePSO = PostProcessing->CreateBloomSunMergePSO(RHIContext);
	GraphicsPSOs.insert(std::make_pair("BloomSunMerge", BloomSunMergePSO));

	IRHIGraphicsPipelineState* CombineLUTsPSO = PostProcessing->CreateCombineLUTsPSO(RHIContext);
	GraphicsPSOs.insert(std::make_pair("CombineLUTs", CombineLUTsPSO));

	IRHIGraphicsPipelineState* ToneMapPSO = PostProcessing->CreateToneMapPSO(RHIContext);
	GraphicsPSOs.insert(std::make_pair("ToneMap", ToneMapPSO));
}

IRHIGraphicsPipelineState* FPSOManager::GetGraphicsPSO(std::string InPSOName)
{
	return GraphicsPSOs[InPSOName];
}

IRHIComputePipelineState* FPSOManager::GeComputePSO(std::string InPSOName)
{
	return ComputePSOs[InPSOName];
}