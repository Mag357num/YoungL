#include "ShadowMap.h"

void FShadowMap::CreateShadowSceneConstant(IRHIContext* InContext)
{
	//initialize scene constant
	FMatrix Proj = Utilities::MatrixPerspectiveFovLH(0.25f * 3.1416f, (1.0f * ShadowVP.Width / ShadowVP.Height), 1.0f, 1000.0f);

	//// Build the initial view matrix.
	FVector4D CamPos = FVector4D(500, 500, 100, 1.0f);
	FVector4D CamTarget = FVector4D(0, 0, 150, 0.0f);
	FVector4D CamUp = FVector4D(0.0f, 0.0f, 1.0f, 0.0f);

	FMatrix View = Utilities::MatrixLookAtLH(CamPos, CamTarget, CamUp);

	SceneConstant.ViewProj = View * Proj;

	//copy to upload buffer transposed???
	SceneConstant.ViewProj = Utilities::MatrixTranspose(SceneConstant.ViewProj);
	SceneConstant.CamLocation = FVector4D(CamPos.X, CamPos.Y, CamPos.Z, 1.0f);

	//create global directional lighting
	SceneConstant.LightStrength = FVector4D(0.5f, 0.5f, 0.5f, 1.0f);
	SceneConstant.LightDirection = FVector4D(-1.0f, -1.0f, -1.0f, 1.0f);

	//Create Scene Constant Buffer
	SceneConstantBuffer = InContext->CreateSceneConstantBuffer(SceneConstant);
}

void FShadowMap::CreateDepthResource(IRHIContext* InContext)
{
	DepthResource = InContext->CreateShadowDepthResource(ShadowVP.Width, ShadowVP.Height, PixelFormat_D24S8);

	//create srv and dsv for depth resource
}