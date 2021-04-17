#pragma once
#include "RHI/RHIContext.h"
#include "RHI/RHIDepthResource.h"

class FShadowMap
{
public:
	FShadowMap(int InWidth, int InHeight)
		:ShadowVP(0, 0, InWidth, InHeight)
	{
		ShadowVP.MinDepth = 0.0f;
		ShadowVP.MaxDepth = 1.0f;
	}
	
	~FShadowMap(){
		if (DepthResource)
		{
			delete DepthResource;
			DepthResource = nullptr;
		}

		if (SceneConstantBuffer)
		{
			delete SceneConstantBuffer;
			SceneConstantBuffer = nullptr;
		}
	}

	void CreateShadowSceneConstant(IRHIContext* InContext, const FBoundSphere& InBound, FVector4D LightDir);

	//test rotate 
	void AutomateRotateLight(const FBoundSphere& InBound);

	void CreateDepthResource(IRHIContext* InContext);

	FViewport* GetViewport(){return &ShadowVP;}
	FRHIDepthResource*GetShadowMapResource(){return DepthResource;}

	IRHIConstantBuffer<FSceneConstant>* GetSceneConstantBuffer(){return SceneConstantBuffer;}

	FMatrix GetLightViewProj(){return Utilities::MatrixTranspose(SceneConstant.ViewProj);}
	FSceneConstant* GetLightSceneConstant(){return &SceneConstant;}

private:
	FViewport ShadowVP;
	FRHIDepthResource* DepthResource;

	//temp for rotate light
	float LightPicth;//Light to XY
	float LightYaw;//Z Rotate

	//save camera info
	FSceneConstant SceneConstant;
	IRHIConstantBuffer<FSceneConstant>* SceneConstantBuffer;
};
