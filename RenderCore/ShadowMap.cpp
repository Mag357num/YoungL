#include "ShadowMap.h"

void FShadowMap::CreateShadowSceneConstant(IRHIContext* InContext, const FBoundSphere& InBound, FVector4D LightDir)
{
	//initialize scene constant//fov is 0.25Pi
	FMatrix Proj = Utilities::MatrixPerspectiveFovLH(0.5f * 3.1416f, 1.0f, 1.0f, 2000.0f);

	float Distance = (float)(InBound.Radius / sin(0.25f * 3.1416f));//fov*1/2
	LightDir = Utilities::Vector3Normalize(LightDir);
	FVector4D LightPos = InBound.Center - LightDir * Distance;

	//// Build the initial view matrix.
	FVector4D CamUp = FVector4D(0.0f, 0.0f, 1.0f, 0.0f);

	FMatrix View = Utilities::MatrixLookAtLH(LightPos, InBound.Center, CamUp);

	SceneConstant.ViewProj = View * Proj;

	//copy to upload buffer transposed???
	SceneConstant.ViewProj = Utilities::MatrixTranspose(SceneConstant.ViewProj);
	SceneConstant.CamLocation = LightPos;

	//create global directional lighting
	SceneConstant.LightStrength = FVector4D(0.5f, 0.5f, 0.5f, 1.0f);
	SceneConstant.LightDirection = FVector4D(-1.0f, -1.0f, -1.0f, 1.0f);

	//Create Scene Constant Buffer
	SceneConstantBuffer = InContext->CreateSceneConstantBuffer(SceneConstant);
}

void FShadowMap::CreateDepthResource(IRHIContext* InContext)
{
	DepthResource = InContext->CreateShadowDepthResource(512, 512, PixelFormat_D24S8);
	//create srv and dsv for depth resource
	InContext->CreateSrvDsvForDepthResource(DepthResource);
}