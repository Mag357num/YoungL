#include "pch.h"
#include "PostProcessing.h"

void FPostProcessing::InitRTs(IRHIContext* Context, int InWidth, int InHeight)
{
	ViewWidth = InWidth;
	ViewHeight = InHeight;

	int BloomSetUpResX = ViewWidth >> 2;
	int BloomSetUpResY = ViewHeight >> 2;
	BloomSetUpResult = Context->CreateColorResource(BloomSetUpResX, BloomSetUpResY, PostProcessFormat);
	Context->CreateSrvRtvForColorResource(BloomSetUpResult);

	int BloomDown0ResX = BloomSetUpResX >> 1;
	int BloomDown0ResY = BloomSetUpResY >> 1;
	BloomDown0 = Context->CreateColorResource(BloomDown0ResX, BloomDown0ResY, PostProcessFormat);
	Context->CreateSrvRtvForColorResource(BloomDown0);

	int BloomDown1ResX = BloomDown0ResX >> 1;
	int BloomDown1ResY = BloomDown0ResY >> 1;
	BloomDown1 = Context->CreateColorResource(BloomDown1ResX, BloomDown1ResY, PostProcessFormat);
	Context->CreateSrvRtvForColorResource(BloomDown1);

	int BloomDown2ResX = BloomDown1ResX >> 1;
	int BloomDown2ResY = BloomDown1ResY >> 1;
	BloomDown2 = Context->CreateColorResource(BloomDown2ResX, BloomDown2ResY, PostProcessFormat);
	Context->CreateSrvRtvForColorResource(BloomDown2);

	int BloomDown3ResX = BloomDown2ResX >> 1;
	int BloomDown3ResY = BloomDown2ResY >> 1;
	BloomDown3 = Context->CreateColorResource(BloomDown3ResX, BloomDown3ResY, PostProcessFormat);
	Context->CreateSrvRtvForColorResource(BloomDown3);

	//bloom up
	BloomUp0 = Context->CreateColorResource(BloomDown2ResX, BloomDown2ResY, PostProcessFormat);
	Context->CreateSrvRtvForColorResource(BloomUp0);

	BloomUp1 = Context->CreateColorResource(BloomDown1ResX, BloomDown1ResY, PostProcessFormat);
	Context->CreateSrvRtvForColorResource(BloomUp1);

	BloomUp2 = Context->CreateColorResource(BloomDown0ResX, BloomDown0ResY, PostProcessFormat);
	Context->CreateSrvRtvForColorResource(BloomUp2);

	SunMerge = Context->CreateColorResource(BloomSetUpResX, BloomSetUpResY, PostProcessFormat);
	Context->CreateSrvRtvForColorResource(SunMerge);

	LUTs = Context->CreateColorResource(1024, 32, LUTFormat);
	Context->CreateSrvRtvForColorResource(LUTs);

	ToneMapResult = Context->CreateColorResource(ViewWidth, ViewHeight, TonemapFormat);
	Context->CreateSrvRtvForColorResource(ToneMapResult);
}

void FPostProcessing::DestroyRTs()
{
	if (BloomSetUpResult)
	{
		delete BloomSetUpResult;
		BloomSetUpResult = nullptr;
	}

	if (BloomDown0)
	{
		delete BloomDown0;
		BloomDown0 = nullptr;
	}

	if (BloomDown1)
	{
		delete BloomDown1;
		BloomDown1 = nullptr;
	}

	if (BloomDown2)
	{
		delete BloomDown2;
		BloomDown2 = nullptr;
	}

	if (BloomDown3)
	{
		delete BloomDown3;
		BloomDown3 = nullptr;
	}

	if (BloomUp0)
	{
		delete BloomUp0;
		BloomUp0 = nullptr;
	}

	if (BloomUp1)
	{
		delete BloomUp1;
		BloomUp1 = nullptr;
	}

	if (BloomUp2)
	{
		delete BloomUp2;
		BloomUp2 = nullptr;
	}

	if (SunMerge)
	{
		delete SunMerge;
		SunMerge = nullptr;
	}

	if (LUTs)
	{
		delete LUTs;
		LUTs = nullptr;
	}

	if (ToneMapResult)
	{
		delete ToneMapResult;
		ToneMapResult = nullptr;
	}
}

IRHIGraphicsPipelineState* FPostProcessing::CreateBloomSetUpPSO(IRHIContext* Context)
{
	IRHIGraphicsPipelineState* BloomSetUpPSO = Context->CreateEmpltyGraphicsPSO();

	FParameterRange ParamRange(RangeType_SRV, 1, 0, 0);
	FRHIShaderParameter ShaderParam(ParaType_Range, 0, 0, Visibility_PS);
	ShaderParam.AddRangeTable(ParamRange);
	BloomSetUpPSO->AddShaderParameter(&ShaderParam);

	FRHIShaderParameter ConstantPara(ParaType_Constant, 0, 0, Visibility_PS);
	ConstantPara.SetNum32BitValues(2);
	BloomSetUpPSO->AddShaderParameter(&ConstantPara);

	BloomSetUpPSO->SetCorlorTargetFormat(PixelFormat_R8G8B8A8_Unorm);

	FRHISamplerState SampleState(0, 0, Filter_MIN_MAG_LINEAR_MIP_POINT, ADDRESS_MODE_CLAMP, ADDRESS_MODE_CLAMP, ADDRESS_MODE_CLAMP);
	BloomSetUpPSO->AddSampleState(&SampleState);

	IRHIShader* VS = new IRHIShader();
	VS->SetShaderType(ShaderType_VS);
	VS->SetShaderPath(L"ScreenVS");
	BloomSetUpPSO->SetVS(VS);

	IRHIShader* PS = new IRHIShader();
	PS->SetShaderType(ShaderType_PS);
	PS->SetShaderPath(L"BloomSetUpPS");
	BloomSetUpPSO->SetPS(PS);

	BloomSetUpPSO->CreateGraphicsPSOInternal();

	return BloomSetUpPSO;
}

IRHIGraphicsPipelineState* FPostProcessing::CreateBloomDownPSO(IRHIContext* Context)
{
	IRHIGraphicsPipelineState* BloomDownPSO = Context->CreateEmpltyGraphicsPSO();

	FParameterRange ParamRange(RangeType_SRV, 1, 0, 0);
	FRHIShaderParameter ShaderParam(ParaType_Range, 0, 0, Visibility_PS);
	ShaderParam.AddRangeTable(ParamRange);
	BloomDownPSO->AddShaderParameter(&ShaderParam);

	FRHIShaderParameter ConstantPara(ParaType_Constant, 0, 0, Visibility_PS);
	ConstantPara.SetNum32BitValues(2);
	BloomDownPSO->AddShaderParameter(&ConstantPara);

	BloomDownPSO->SetCorlorTargetFormat(PixelFormat_R8G8B8A8_Unorm);

	FRHISamplerState SampleState(0, 0, Filter_MIN_MAG_LINEAR_MIP_POINT, ADDRESS_MODE_CLAMP, ADDRESS_MODE_CLAMP, ADDRESS_MODE_CLAMP);
	BloomDownPSO->AddSampleState(&SampleState);

	IRHIShader* VS = new IRHIShader();
	VS->SetShaderType(ShaderType_VS);
	VS->SetShaderPath(L"ScreenVS");
	BloomDownPSO->SetVS(VS);

	IRHIShader* PS = new IRHIShader();
	PS->SetShaderType(ShaderType_PS);
	PS->SetShaderPath(L"BloomDownPS");
	BloomDownPSO->SetPS(PS);

	BloomDownPSO->CreateGraphicsPSOInternal();

	return BloomDownPSO;
}
IRHIGraphicsPipelineState* FPostProcessing::CreateBloomUpPSO(IRHIContext* Context)
{
	IRHIGraphicsPipelineState* BloomUpPSO = Context->CreateEmpltyGraphicsPSO();

	FParameterRange ParamRange(RangeType_SRV, 1, 0, 0);
	FRHIShaderParameter ShaderParam(ParaType_Range, 0, 0, Visibility_PS);
	ShaderParam.AddRangeTable(ParamRange);
	BloomUpPSO->AddShaderParameter(&ShaderParam);

	FRHIShaderParameter ConstantPara(ParaType_Constant, 0, 0, Visibility_PS);
	ConstantPara.SetNum32BitValues(2);
	BloomUpPSO->AddShaderParameter(&ConstantPara);

	BloomUpPSO->SetCorlorTargetFormat(PixelFormat_R8G8B8A8_Unorm);

	FRHISamplerState SampleState(0, 0, Filter_MIN_MAG_LINEAR_MIP_POINT, ADDRESS_MODE_CLAMP, ADDRESS_MODE_CLAMP, ADDRESS_MODE_CLAMP);
	BloomUpPSO->AddSampleState(&SampleState);

	IRHIShader* VS = new IRHIShader();
	VS->SetShaderType(ShaderType_VS);
	VS->SetShaderPath(L"ScreenVS");
	BloomUpPSO->SetVS(VS);

	IRHIShader* PS = new IRHIShader();
	PS->SetShaderType(ShaderType_PS);
	PS->SetShaderPath(L"BloomUpPS");
	BloomUpPSO->SetPS(PS);

	BloomUpPSO->CreateGraphicsPSOInternal();

	return BloomUpPSO;
}
IRHIGraphicsPipelineState* FPostProcessing::CreateCombineLUTsPSO(IRHIContext* Context)
{
	IRHIGraphicsPipelineState* CombineLUTsPSO = Context->CreateEmpltyGraphicsPSO();

	FParameterRange ParamRange(RangeType_SRV, 1, 0, 0);
	FRHIShaderParameter ShaderParam(ParaType_Range, 0, 0, Visibility_PS);
	ShaderParam.AddRangeTable(ParamRange);
	CombineLUTsPSO->AddShaderParameter(&ShaderParam);

	FRHIShaderParameter ConstantPara(ParaType_Constant, 0, 0, Visibility_PS);
	ConstantPara.SetNum32BitValues(2);
	CombineLUTsPSO->AddShaderParameter(&ConstantPara);

	CombineLUTsPSO->SetCorlorTargetFormat(PixelFormat_R8G8B8A8_Unorm);

	FRHISamplerState SampleState(0, 0, Filter_MIN_MAG_LINEAR_MIP_POINT, ADDRESS_MODE_CLAMP, ADDRESS_MODE_CLAMP, ADDRESS_MODE_CLAMP);
	CombineLUTsPSO->AddSampleState(&SampleState);

	IRHIShader* VS = new IRHIShader();
	VS->SetShaderType(ShaderType_VS);
	VS->SetShaderPath(L"ScreenVS");
	CombineLUTsPSO->SetVS(VS);

	IRHIShader* PS = new IRHIShader();
	PS->SetShaderType(ShaderType_PS);
	PS->SetShaderPath(L"CombineLUTsPS");
	CombineLUTsPSO->SetPS(PS);

	CombineLUTsPSO->CreateGraphicsPSOInternal();

	return CombineLUTsPSO;
}
IRHIGraphicsPipelineState* FPostProcessing::CreateToneMapPSO(IRHIContext* Context)
{
	IRHIGraphicsPipelineState* ToneMapPSO = Context->CreateEmpltyGraphicsPSO();

	FParameterRange ParamRange(RangeType_SRV, 1, 0, 0);
	FRHIShaderParameter ShaderParam(ParaType_Range, 0, 0, Visibility_PS);
	ShaderParam.AddRangeTable(ParamRange);
	ToneMapPSO->AddShaderParameter(&ShaderParam);

	FRHIShaderParameter ConstantPara(ParaType_Constant, 0, 0, Visibility_PS);
	ConstantPara.SetNum32BitValues(2);
	ToneMapPSO->AddShaderParameter(&ConstantPara);

	ToneMapPSO->SetCorlorTargetFormat(PixelFormat_R8G8B8A8_Unorm);

	FRHISamplerState SampleState(0, 0, Filter_MIN_MAG_LINEAR_MIP_POINT, ADDRESS_MODE_CLAMP, ADDRESS_MODE_CLAMP, ADDRESS_MODE_CLAMP);
	ToneMapPSO->AddSampleState(&SampleState);

	IRHIShader* VS = new IRHIShader();
	VS->SetShaderType(ShaderType_VS);
	VS->SetShaderPath(L"ScreenVS");
	ToneMapPSO->SetVS(VS);

	IRHIShader* PS = new IRHIShader();
	PS->SetShaderType(ShaderType_PS);
	PS->SetShaderPath(L"ToneMapPS");
	ToneMapPSO->SetPS(PS);

	ToneMapPSO->CreateGraphicsPSOInternal();

	return ToneMapPSO;
}

void FPostProcessing::BloomSetUp(FRHIColorResource* SceneColor)
{

}

void FPostProcessing::BloomDown()
{

}

void FPostProcessing::BloomUp()
{

}

void FPostProcessing::CombineLUTs()
{

}

void FPostProcessing::ToneMap()
{

}