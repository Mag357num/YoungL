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
	}

	~FPostProcessing(){}

	void InitRTs(IRHIContext* Context, int InWidth, int InHeight);
	void DestroyRTs();


	IRHIGraphicsPipelineState* CreateBloomSetUpPSO(IRHIContext* Context);
	IRHIGraphicsPipelineState* CreateBloomDownPSO(IRHIContext* Context);
	IRHIGraphicsPipelineState* CreateBloomUpPSO(IRHIContext* Context);
	IRHIGraphicsPipelineState* CreateCombineLUTsPSO(IRHIContext* Context);
	IRHIGraphicsPipelineState* CreateToneMapPSO(IRHIContext* Context);

	void BloomSetUp(FRHIColorResource* SceneColor);
	void BloomDown();
	void BloomUp();
	void CombineLUTs();
	void ToneMap();

	FRHIColorResource* GetToneMapResult() {
		return ToneMapResult;
		}

private:
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
	EPixelBufferFormat LUTFormat;
	EPixelBufferFormat TonemapFormat;

	int ViewWidth;
	int ViewHeight;
};
