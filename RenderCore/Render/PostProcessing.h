#pragma once
#include "RHI/RHIContext.h"

class FPostProcessing
{
public:
	FPostProcessing()
	{
		PostProcessFormat = PixelFormat_R11G11B10_Float;
		LUTFormat = PixelFormat_R10G10B10A2_UNorm;
		TonemapFormat = PixelFormat_R8G8B8A8_Unorm;
		SunMergeFormat = PixelFormat_R16G16B16A16_Float;
	}

	~FPostProcessing(){}

	void InitRTs(IRHIContext* Context, int InWidth, int InHeight);
	void DestroyRTs();


	IRHIGraphicsPipelineState* CreateBloomSetUpPSO(IRHIContext* Context);
	IRHIGraphicsPipelineState* CreateBloomDownPSO(IRHIContext* Context);
	IRHIGraphicsPipelineState* CreateBloomUpPSO(IRHIContext* Context);
	IRHIGraphicsPipelineState* CreateBloomSunMergePSO(IRHIContext* Context);
	IRHIGraphicsPipelineState* CreateCombineLUTsPSO(IRHIContext* Context);
	IRHIGraphicsPipelineState* CreateToneMapPSO(IRHIContext* Context);

	void BloomSetUp(IRHIContext* Context, FRHIColorResource* SceneColor, IRHIGraphicsPipelineState* PSO);
	void BloomDown(IRHIContext* Context, IRHIGraphicsPipelineState* PSO, UINT Stage = 0);
	void BloomUp(IRHIContext* Context, IRHIGraphicsPipelineState* PSO, UINT Stage = 0);
	void BloomSunMerge(IRHIContext* Context, IRHIGraphicsPipelineState* PSO);
	void CombineLUTs(IRHIContext* Context, IRHIGraphicsPipelineState* PSO);
	void ToneMap(IRHIContext* Context, FRHIColorResource* SceneColor, IRHIGraphicsPipelineState* PSO);

	FRHIColorResource* GetToneMapResult() {
		return ToneMapResult;
		}


private:

	FRHIColorResource* FindBloomTargetByStage(bool IsBloomDown, UINT Stage);
	FRHIColorResource* FindBloomInputByStage(bool IsBloomDown, UINT Stage);
	FRHIColorResource* FindBloomUpSecondInputByStage(UINT Stage);

	void GetBloomInputSizeByStage(int& Width, int& Height, bool IsBloomDown, UINT Stage);

	//scene color
	FRHIColorResource* BloomSetUpResult;
	FRHIColorResource* BloomDown0;
	FRHIColorResource* BloomDown1;
	FRHIColorResource* BloomDown2;
	FRHIColorResource* BloomDown3;

	FRHIColorResource* BloomUp0;
	FRHIColorResource* BloomUp1;
	FRHIColorResource* BloomUp2;
	FRHIColorResource* SunMerge;

	FRHIColorResource* LUTs;

	FRHIColorResource* ToneMapResult;

	EPixelBufferFormat PostProcessFormat;
	EPixelBufferFormat SunMergeFormat;
	EPixelBufferFormat LUTFormat;
	EPixelBufferFormat TonemapFormat;

	int ViewWidth;
	int ViewHeight;
};
