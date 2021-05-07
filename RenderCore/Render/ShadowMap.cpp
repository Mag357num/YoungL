#include "pch.h"
#include "ShadowMap.h"

void FShadowMap::CreateShadowSceneConstant(IRHIContext* InContext, const FBoundSphere& InBound, FVector4D LightDir)
{
	//calc and save rot and pitch
	LightPicth = (float)atan(-LightDir.Z / sqrt(LightDir.X * LightDir.X + LightDir.Y * LightDir.Y));
	LightYaw = (float)atan(LightDir.Y / LightDir.X);

	//initialize scene constant//fov is 0.25Pi
	FMatrix Proj = FMath::MatrixPerspectiveFovLH(0.5f * 3.1416f, 1.0f, 1.0f, 2000.0f);

	float Distance = (float)(InBound.Radius / sin(0.25f * 3.1416f));//fov*1/2
	LightDir = FMath::Vector3Normalize(LightDir);
	FVector4D LightPos = InBound.Center - LightDir * Distance;

	//// Build the initial view matrix.
	FVector4D CamUp = FVector4D(0.0f, 0.0f, 1.0f, 0.0f);

	FMatrix View = FMath::MatrixLookAtLH(LightPos, InBound.Center, CamUp);

	SceneConstant.ViewProj = View * Proj;

	//copy to upload buffer transposed???
	SceneConstant.ViewProj = FMath::MatrixTranspose(SceneConstant.ViewProj);
	SceneConstant.CamLocation = LightPos;

	//create global directional lighting
	SceneConstant.LightStrength = FVector4D(0.5f, 0.5f, 0.5f, 1.0f);
	SceneConstant.LightDirection = FVector4D(-1.0f, -1.0f, -1.0f, 1.0f);

	//Create Scene Constant Buffer
	SceneConstantBuffer = InContext->CreateSceneConstantBuffer(SceneConstant);
}

void FShadowMap::AutomateRotateLight(const FBoundSphere& InBound)
{
	//recover light direction from pitch and yaw
	LightYaw += 0.01f;
	//reset to 0~2*PI
	int LightYar100 = (int)round(LightYaw * 100);
	int Mod = LightYar100 % 628;
	LightYaw = Mod * 0.01f;
	//
	FVector4D LightDir = FVector4D((float)cos(LightYaw), (float)sin(LightYaw), (float)tan(LightPicth), 0.0f);

	//initialize scene constant//fov is 0.25Pi
	FMatrix Proj = FMath::MatrixPerspectiveFovLH(0.5f * 3.1416f, 1.0f, 1.0f, 2000.0f);

	float Distance = (float)(InBound.Radius / sin(0.25f * 3.1416f));//fov*1/2
	LightDir = FMath::Vector3Normalize(LightDir);
	FVector4D LightPos = InBound.Center + LightDir * Distance;

	//// Build the initial view matrix.
	FVector4D CamUp = FVector4D(0.0f, 0.0f, 1.0f, 0.0f);

	FMatrix View = FMath::MatrixLookAtLH(LightPos, InBound.Center, CamUp);

	SceneConstant.ViewProj = View * Proj;

	//copy to upload buffer transposed???
	SceneConstant.ViewProj = FMath::MatrixTranspose(SceneConstant.ViewProj);
	SceneConstant.CamLocation = LightPos;

	//create global directional lighting
	SceneConstant.LightStrength = FVector4D(0.8f, 0.8f, 0.8f, 1.0f);
	//update newst light direction 
	SceneConstant.LightDirection = FVector4D(-LightDir.X, -LightDir.Y, -LightDir.Z, 0.0f);

	//copy data
	SceneConstantBuffer->CopyData(0, SceneConstant);
}

void FShadowMap::CreateDepthResource(IRHIContext* InContext)
{
	DepthResource = InContext->CreateDepthResource(512, 512, PixelFormat_R24G8_Typeless);
	//create srv and dsv for depth resource
	InContext->CreateSrvDsvForDepthResource(DepthResource);
}