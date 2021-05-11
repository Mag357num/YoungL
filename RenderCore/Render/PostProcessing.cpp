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

	SunMerge = Context->CreateColorResource(BloomSetUpResX, BloomSetUpResY, SunMergeFormat);
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

	BloomSetUpPSO->SetCorlorTargetFormat(PostProcessFormat);

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

	BloomDownPSO->SetCorlorTargetFormat(PostProcessFormat);

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

	FParameterRange ParamRange1(RangeType_SRV, 1, 1, 0);
	FRHIShaderParameter ShaderParam1(ParaType_Range, 0, 0, Visibility_PS);
	ShaderParam1.AddRangeTable(ParamRange1);
	BloomUpPSO->AddShaderParameter(&ShaderParam1);

	FRHIShaderParameter ConstantPara(ParaType_Constant, 0, 0, Visibility_PS);
	ConstantPara.SetNum32BitValues(4);
	BloomUpPSO->AddShaderParameter(&ConstantPara);

	BloomUpPSO->SetCorlorTargetFormat(PostProcessFormat);

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

IRHIGraphicsPipelineState* FPostProcessing::CreateBloomSunMergePSO(IRHIContext* Context)
{
	IRHIGraphicsPipelineState* BloomSunMergePSO = Context->CreateEmpltyGraphicsPSO();

	FParameterRange ParamRange(RangeType_SRV, 1, 0, 0);
	FRHIShaderParameter ShaderParam(ParaType_Range, 0, 0, Visibility_PS);
	ShaderParam.AddRangeTable(ParamRange);
	BloomSunMergePSO->AddShaderParameter(&ShaderParam);

	FParameterRange ParamRange1(RangeType_SRV, 1, 1, 0);
	FRHIShaderParameter ShaderParam1(ParaType_Range, 0, 0, Visibility_PS);
	ShaderParam1.AddRangeTable(ParamRange1);
	BloomSunMergePSO->AddShaderParameter(&ShaderParam1);

	FRHIShaderParameter ConstantPara(ParaType_Constant, 0, 0, Visibility_PS);
	ConstantPara.SetNum32BitValues(4);
	BloomSunMergePSO->AddShaderParameter(&ConstantPara);

	BloomSunMergePSO->SetCorlorTargetFormat(SunMergeFormat);

	FRHISamplerState SampleState(0, 0, Filter_MIN_MAG_LINEAR_MIP_POINT, ADDRESS_MODE_CLAMP, ADDRESS_MODE_CLAMP, ADDRESS_MODE_CLAMP);
	BloomSunMergePSO->AddSampleState(&SampleState);

	IRHIShader* VS = new IRHIShader();
	VS->SetShaderType(ShaderType_VS);
	VS->SetShaderPath(L"ScreenVS");
	BloomSunMergePSO->SetVS(VS);

	IRHIShader* PS = new IRHIShader();
	PS->SetShaderType(ShaderType_PS);
	PS->SetShaderPath(L"BloomSunMergePS");
	BloomSunMergePSO->SetPS(PS);

	BloomSunMergePSO->CreateGraphicsPSOInternal();

	return BloomSunMergePSO;
}



IRHIGraphicsPipelineState* FPostProcessing::CreateCombineLUTsPSO(IRHIContext* Context)
{
	IRHIGraphicsPipelineState* CombineLUTsPSO = Context->CreateEmpltyGraphicsPSO();


	FRHIShaderParameter ConstantPara(ParaType_Constant, 0, 0, Visibility_PS);
	ConstantPara.SetNum32BitValues(2);
	CombineLUTsPSO->AddShaderParameter(&ConstantPara);

	CombineLUTsPSO->SetCorlorTargetFormat(LUTFormat);

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


	FParameterRange ParamRange1(RangeType_SRV, 1, 1, 0);
	FRHIShaderParameter ShaderParam1(ParaType_Range, 0, 0, Visibility_PS);
	ShaderParam1.AddRangeTable(ParamRange1);
	ToneMapPSO->AddShaderParameter(&ShaderParam1);

	FRHIShaderParameter ConstantPara(ParaType_Constant, 0, 0, Visibility_PS);
	ConstantPara.SetNum32BitValues(4);
	ToneMapPSO->AddShaderParameter(&ConstantPara);

	ToneMapPSO->SetCorlorTargetFormat(TonemapFormat);

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

void FPostProcessing::BloomSetUp(IRHIContext* Context, FRHIColorResource* SceneColor, IRHIGraphicsPipelineState* PSO)
{
	Context->BeginEvent(L"Bloom Setup");

	FViewport Viewport;
	Viewport.X = 0;
	Viewport.Y = 0;
	Viewport.Width = (int)round(ViewWidth * 0.25f);
	Viewport.Height = (int)round(ViewHeight * 0.25f);
	Viewport.MinDepth = 0.0f;
	Viewport.MaxDepth = 1.0f;

	Context->SetViewport(Viewport);
	Context->SetScissor(0, 0, (long)Viewport.Width, (long)Viewport.Height);

	//change back buffer state to rendertarget
	Context->TransitionResource(BloomSetUpResult, State_GenerateRead, State_RenderTarget);
	Context->SetColorTarget(BloomSetUpResult);
	Context->SetGraphicsPipilineState(PSO);
	Context->PreparePresentShaderParameter();

	//set root constant
	Context->SetGraphicRootConstant(1, ViewWidth, 0);
	//set root constant
	Context->SetGraphicRootConstant(1, ViewHeight, 1);

	Context->SetColorSRV(0, SceneColor);
	Context->Draw(3);

	//	//change back buffer state to present
	Context->TransitionResource(BloomSetUpResult, State_RenderTarget, State_GenerateRead);

	Context->EndEvent();
}

void FPostProcessing::BloomDown(IRHIContext* Context, IRHIGraphicsPipelineState* PSO, UINT Stage)
{
	Context->BeginEvent(L"Bloom Down");

	int InputWidth, InputHeight;
	GetBloomInputSizeByStage(InputWidth, InputHeight, true, Stage);

	FViewport Viewport;
	Viewport.X = 0;
	Viewport.Y = 0;
	Viewport.Width = (int)round(InputWidth * 0.5f);
	Viewport.Height = (int)round(InputHeight * 0.5f);
	Viewport.MinDepth = 0.0f;
	Viewport.MaxDepth = 1.0f;

	Context->SetViewport(Viewport);
	Context->SetScissor(0, 0, (long)Viewport.Width, (long)Viewport.Height);

	//change back buffer state to rendertarget
	FRHIColorResource* RenderTarget = FindBloomTargetByStage(true, Stage);
	Context->TransitionResource(RenderTarget, State_GenerateRead, State_RenderTarget);
	Context->SetColorTarget(RenderTarget);
	
	
	Context->SetGraphicsPipilineState(PSO);
	Context->PreparePresentShaderParameter();

	//set root constant
	Context->SetGraphicRootConstant(1, InputWidth, 0);
	//set root constant
	Context->SetGraphicRootConstant(1, InputHeight, 1);

	FRHIColorResource* InputResource = FindBloomInputByStage(true, Stage);
	Context->SetColorSRV(0, InputResource);
	
	
	Context->Draw(3);

	//	//change back buffer state to present
	Context->TransitionResource(RenderTarget, State_RenderTarget, State_GenerateRead);

	Context->EndEvent();
}

void FPostProcessing::BloomUp(IRHIContext* Context, IRHIGraphicsPipelineState* PSO, UINT Stage)
{
	Context->BeginEvent(L"Bloom Up");

	int InputWidth, InputHeight;
	GetBloomInputSizeByStage(InputWidth, InputHeight, false, Stage);

	FViewport Viewport;
	Viewport.X = 0;
	Viewport.Y = 0;
	Viewport.Width = (int)round(InputWidth);
	Viewport.Height = (int)round(InputHeight);
	Viewport.MinDepth = 0.0f;
	Viewport.MaxDepth = 1.0f;

	Context->SetViewport(Viewport);
	Context->SetScissor(0, 0, (long)Viewport.Width, (long)Viewport.Height);

	//change back buffer state to rendertarget
	FRHIColorResource* RenderTarget = FindBloomTargetByStage(false, Stage);
	Context->TransitionResource(RenderTarget, State_GenerateRead, State_RenderTarget);
	Context->SetColorTarget(RenderTarget);


	Context->SetGraphicsPipilineState(PSO);
	Context->PreparePresentShaderParameter();

	//set root constant
	Context->SetGraphicRootConstant(2, InputWidth, 0);
	Context->SetGraphicRootConstant(2, InputHeight, 1);

	if (Stage == 0)
	{
		Context->SetGraphicRootConstant(2, 42, 2);
		Context->SetGraphicRootConstant(2, 42, 3);
	}
	else if (Stage == 1)
	{
		Context->SetGraphicRootConstant(2, 8, 2);
		Context->SetGraphicRootConstant(2, 23, 3);
	}
	else if (Stage == 2)
	{
		Context->SetGraphicRootConstant(2, 8, 2);
		Context->SetGraphicRootConstant(2, 40, 3);
	}
	else
	{
		Context->SetGraphicRootConstant(2, 8, 2);
		Context->SetGraphicRootConstant(2, 40, 3);
	}
	

	FRHIColorResource* InputResource = FindBloomInputByStage(false, Stage);
	Context->SetColorSRV(0, InputResource);

	FRHIColorResource* InputResource1 = FindBloomUpSecondInputByStage(Stage);
	Context->SetColorSRV(1, InputResource1);


	Context->Draw(3);

	//	//change back buffer state to present
	Context->TransitionResource(RenderTarget, State_RenderTarget, State_GenerateRead);

	Context->EndEvent();
}

void FPostProcessing::BloomSunMerge(IRHIContext* Context, IRHIGraphicsPipelineState* PSO)
{
	Context->BeginEvent(L"Bloom SunMerge");

	int InputWidth, InputHeight;
	GetBloomInputSizeByStage(InputWidth, InputHeight, false, 3);

	FViewport Viewport;
	Viewport.X = 0;
	Viewport.Y = 0;
	Viewport.Width = (int)round(InputWidth);
	Viewport.Height = (int)round(InputHeight);
	Viewport.MinDepth = 0.0f;
	Viewport.MaxDepth = 1.0f;

	Context->SetViewport(Viewport);
	Context->SetScissor(0, 0, (long)Viewport.Width, (long)Viewport.Height);

	//change back buffer state to rendertarget
	FRHIColorResource* RenderTarget = FindBloomTargetByStage(false, 3);
	Context->TransitionResource(RenderTarget, State_GenerateRead, State_RenderTarget);
	Context->SetColorTarget(RenderTarget);


	Context->SetGraphicsPipilineState(PSO);
	Context->PreparePresentShaderParameter();

	//set root constant
	Context->SetGraphicRootConstant(2, InputWidth, 0);
	Context->SetGraphicRootConstant(2, InputHeight, 1);

	Context->SetGraphicRootConstant(2, 8, 2);
	Context->SetGraphicRootConstant(2, 40, 3);


	FRHIColorResource* InputResource = FindBloomInputByStage(false, 3);
	Context->SetColorSRV(0, InputResource);

	FRHIColorResource* InputResource1 = FindBloomUpSecondInputByStage(3);
	Context->SetColorSRV(1, InputResource1);


	Context->Draw(3);

	//	//change back buffer state to present
	Context->TransitionResource(RenderTarget, State_RenderTarget, State_GenerateRead);

	Context->EndEvent();
}

void FPostProcessing::CombineLUTs(IRHIContext* Context, IRHIGraphicsPipelineState* PSO)
{
	Context->BeginEvent(L"Combine LUTs");


	FViewport Viewport;
	Viewport.X = 0;
	Viewport.Y = 0;
	Viewport.Width = 1024;
	Viewport.Height = 32;
	Viewport.MinDepth = 0.0f;
	Viewport.MaxDepth = 1.0f;

	Context->SetViewport(Viewport);
	Context->SetScissor(0, 0, (long)Viewport.Width, (long)Viewport.Height);

	//change back buffer state to rendertarget
	Context->TransitionResource(LUTs, State_GenerateRead, State_RenderTarget);
	Context->SetColorTarget(LUTs);


	Context->SetGraphicsPipilineState(PSO);
	Context->PreparePresentShaderParameter();

	//set root constant
	Context->SetGraphicRootConstant(0, 1024, 0);
	Context->SetGraphicRootConstant(0, 32, 1);


	Context->Draw(3);

	//	//change back buffer state to present
	Context->TransitionResource(LUTs, State_RenderTarget, State_GenerateRead);

	Context->EndEvent();
}

void FPostProcessing::ToneMap(IRHIContext* Context, FRHIColorResource* SceneColor, IRHIGraphicsPipelineState* PSO)
{
	Context->BeginEvent(L"ToneMap");

	FViewport Viewport;
	Viewport.X = 0;
	Viewport.Y = 0;
	Viewport.Width = ViewWidth;
	Viewport.Height = ViewHeight;
	Viewport.MinDepth = 0.0f;
	Viewport.MaxDepth = 1.0f;

	Context->SetViewport(Viewport);
	Context->SetScissor(0, 0, (long)Viewport.Width, (long)Viewport.Height);

	//change back buffer state to rendertarget
	Context->TransitionBackBufferStateToRT();
	//set backbuffer as rendertarget
	Context->SetBackBufferAsRt();


	Context->SetGraphicsPipilineState(PSO);
	Context->PreparePresentShaderParameter();

	//set root constant
	Context->SetGraphicRootConstant(2, ViewWidth, 0);
	Context->SetGraphicRootConstant(2, ViewHeight, 1);

	Context->SetColorSRV(0, SceneColor);
	Context->SetColorSRV(1, SunMerge);

	Context->Draw(3);

	//	//change back buffer state to present
	Context->TransitionBackBufferStateToPresent();

	Context->EndEvent();
}

FRHIColorResource* FPostProcessing::FindBloomTargetByStage(bool IsBloomDown, UINT Stage)
{
	FRHIColorResource* Ret= BloomDown0;

	if (IsBloomDown)
	{
		switch (Stage)
		{
		case 0:
			Ret = BloomDown0;
			break;
		case 1:
			Ret = BloomDown1;
			break;
		case 2:
			Ret = BloomDown2;
			break;
		case 3:
			Ret = BloomDown3;
			break;
		default:
			break;
		}
	}
	else
	{
		switch (Stage)
		{
		case 0:
			Ret = BloomUp0;
			break;
		case 1:
			Ret = BloomUp1;
			break;
		case 2:
			Ret = BloomUp2;
			break;
		case 3:
			Ret = SunMerge;
			break;
		default:
			break;
		}
	}

	return Ret;
}

FRHIColorResource* FPostProcessing::FindBloomInputByStage(bool IsBloomDown, UINT Stage)
{
	FRHIColorResource* Ret = BloomSetUpResult;

	if (IsBloomDown)
	{
		switch (Stage)
		{
		case 0:
			Ret = BloomSetUpResult;
			break;
		case 1:
			Ret = BloomDown0;
			break;
		case 2:
			Ret = BloomDown1;
			break;
		case 3:
			Ret = BloomDown2;
			break;
		default:
			break;
		}
	}
	else
	{
		switch (Stage)
		{
		case 0:
			Ret = BloomDown3;//and bloom Down2
			break;
		case 1:
			Ret = BloomUp0; //and bloom down 1
			break;
		case 2:
			Ret = BloomUp1;//and bloom down 0
			break;
		case 3:
			Ret = BloomUp2;// bloom set up
			break;
		default:
			break;
		}
	}

	return Ret;
}

FRHIColorResource* FPostProcessing::FindBloomUpSecondInputByStage(UINT Stage)
{
	FRHIColorResource* Ret = BloomDown2;

	switch (Stage)
	{
	case 0:
		Ret = BloomDown2;//and bloom Down2
		break;
	case 1:
		Ret = BloomDown1; //and bloom down 1
		break;
	case 2:
		Ret = BloomDown0;//and bloom down 0
		break;
	case 3:
		Ret = BloomSetUpResult;// bloom set up
		break;
	default:
		break;
	}

	return Ret;
}

void FPostProcessing::GetBloomInputSizeByStage(int& Width, int& Height, bool IsBloomDown, UINT Stage)
{
	if (IsBloomDown)
	{
		switch (Stage)
		{
		case 0:
			Width = ViewWidth >> 2;
			Height = ViewHeight >> 2;
			break;

		case 1:
			Width = ViewWidth >> 3;
			Height = ViewHeight >> 3;
			break;

		case 2:
			Width = ViewWidth >> 4;
			Height = ViewHeight >> 4;
			break;

		case 3:
			Width = ViewWidth >> 5;
			Height = ViewHeight >> 5;
			break;

		default:
			break;
		}
	}
	else
	{
		switch (Stage)
		{
		case 0:
			Width = ViewWidth >> 5;
			Height = ViewHeight >> 5;
			break;

		case 1:
			Width = ViewWidth >> 4;
			Height = ViewHeight >> 4;
			break;

		case 2:
			Width = ViewWidth >> 3;
			Height = ViewHeight >> 3;
			break;

		case 3:
			Width = ViewWidth >> 2;
			Height = ViewHeight >> 2;
			break;

		default:
			break;
		}
	}
}