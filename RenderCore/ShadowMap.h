#pragma once
#include "RHI/RHIContext.h"
#include "RHI/RHIDepthResource.h"

class FShadowMap
{
public:
	FShadowMap(int InWidth, int InHeight)
		:ShadowVP(0, 0, InWidth, InHeight)
	{}
	
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

	void CreateShadowSceneConstant(IRHIContext* InContext);

	void CreateDepthResource(IRHIContext* InContext);



private:
	FViewport ShadowVP;
	FRHIDepthResource* DepthResource;

	//save camera info
	FSceneConstant SceneConstant;
	IRHIConstantBuffer<FSceneConstant>* SceneConstantBuffer;
};
