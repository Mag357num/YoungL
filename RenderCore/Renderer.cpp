#include "pch.h"
#include "Renderer.h"
#include "RHI/D3D12/RHIContext_D3D12.h"

#define MAX_LOADSTRING 100

using namespace Utilities;

void FRenderer::CreateRHIContext(int InWidth, int Inheight)
{
#ifdef _WIN32
	RHIContext = new FRHIContext_D3D12();
#elif __APPLE__

#elif __ANDROID__

#else

#endif

	//viewport
	Viewport.X = 0;
	Viewport.Y = 0;
	Viewport.Width = InWidth;
	Viewport.Height = Inheight;
	Viewport.MaxDepth = 1.0f;
	Viewport.MinDepth = 0.0f;

	RHIContext->InitializeRHI(InWidth, Inheight);
	IRHIGraphicsPipelineState* BasePassPSO = RHIContext->CreateGraphicsPSO();
	GraphicsPSOs.insert(std::make_pair("BasePass", BasePassPSO));

	//initialize scene constant
	FMatrix Proj = Utilities::MatrixPerspectiveFovLH(0.25f * 3.1416f, (1.0f * Viewport.Width / Viewport.Height), 1.0f, 1000.0f);

	//// Build the initial view matrix.
	FVector4D CamPos = FVector4D(500, 500, 100, 0.0f);
	FVector4D CamTarget = FVector4D(0, 0, 150, 0.0f);
	FVector4D CamUp = FVector4D(0.0f, 0.0f, 1.0f, 0.0f);

	FMatrix View = Utilities::MatrixLookAtLH(CamPos, CamTarget, CamUp);

	SceneConstant.ViewProj = View * Proj;

	//copy to upload buffer transposed???
	SceneConstant.ViewProj = Utilities::MatrixTranspose(SceneConstant.ViewProj);
	SceneConstant.CamLocation = FVector4D(CamPos.X, CamPos.Y, CamPos.Z, 0.0f);
	
	//create global directional lighting
	SceneConstant.LightStrength = FVector4D(1.0f, 1.0f, 1.0f, 0.0f);
	SceneConstant.LightDirection = FVector4D(-1.0f, -1.0f, -1.0f, 0.0f);


	//create shadow map
	if (!ShadowMap)
	{
		ShadowMap = new FShadowMap(512, 512);
		FBoundSphere Bound;
		Bound.Center = FVector4D(0.0f, 0.0f, 0.0f, 0.0f);
		Bound.Radius = 500.0f;
		ShadowMap->CreateDepthResource(RHIContext);
		ShadowMap->CreateShadowSceneConstant(RHIContext, Bound, SceneConstant.LightDirection);

		//set scene constants 
		FMatrix LightVP = ShadowMap->GetLightViewProj();
		// Transform NDC space [-1,+1]^2 to texture space [0,1]^2
		FMatrix T(
			0.5f, 0.0f, 0.0f, 0.0f,
			0.0f, -0.5f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.5f, 0.5f, 0.0f, 1.0f);

		SceneConstant.LightViewProj = LightVP * T;
		SceneConstant.LightViewProj = Utilities::MatrixTranspose(SceneConstant.LightViewProj);

		IRHIGraphicsPipelineState* DepthPassPSO = RHIContext->CreateGraphicsDepthPSO();
		GraphicsPSOs.insert(std::make_pair("DepthPass", DepthPassPSO));
	}

	//Create Scene Constant Buffer
	SceneConstantBuffer = RHIContext->CreateSceneConstantBuffer(SceneConstant);
}

void FRenderer::DestroyRHIContext()
{
	//release pso state object
	for (auto It=GraphicsPSOs.begin(); It != GraphicsPSOs.end(); ++It)
	{
		delete It->second;
		It->second = nullptr;
	}
	//empty
	if (!GraphicsPSOs.empty())
	{
		printf("Empty Error!");
	}

	if (SceneConstantBuffer)
	{
		delete SceneConstantBuffer;
		SceneConstantBuffer = nullptr;
	}

	for (int ItemIndex = 0; ItemIndex < RenderingMeshes.size(); ItemIndex++)
	{
		RenderingMeshes[ItemIndex]->Release();
		delete RenderingMeshes[ItemIndex];
	}

	if (!RenderingMeshes.empty())
	{
		printf("Empty Error!");
	}
	
	//shadowmap
	if (ShadowMap)
	{
		delete ShadowMap;
		ShadowMap = nullptr;
	}

#ifdef _WIN32
	delete RHIContext;
	RHIContext = nullptr;

#elif __APPLE__

#elif __ANDROID__

#else

#endif

}

void FRenderer::Resize(int InWidth, int InHeight)
{
	if (RHIContext)
	{
		RHIContext->Resize(InWidth, InHeight);
	}
}

void FRenderer::CreateRenderingItem(std::vector<std::unique_ptr<AMeshActor>>& Geometries)
{
	for (int Index = 0; Index < Geometries.size(); ++Index)
	{
		//create an empty rendering item

		IRHIRenderingMesh* Item = RHIContext->CreateEmptyRenderingMesh();

		Item->BuildConstantBuffer(Geometries[Index]->GetObjectConstants(), RHIContext);
		Item->BuildVertexBuffer(Geometries[Index]->GetGeometry()->GetVertices());
		Item->BuildIndexBuffer(Geometries[Index]->GetGeometry()->GetIndices());

		RenderingMeshes.push_back(Item);
	}

	//buil scene constant buffer
}

void FRenderer::RenderObjects()
{
	//reset command list and command allocator here
	RHIContext->BeginDraw(L"BasePass");

	//render depth map first
	//for realtime shadow
	RenderDepth();

	RHIContext->SetViewport(Viewport);
	RHIContext->SetScissor(0, 0, (long)Viewport.Width, (long)Viewport.Height);

	//change back buffer state to rendertarget
	RHIContext->TransitionBackBufferStateToRT();

	//set backbuffer as rendertarget
	RHIContext->SetBackBufferAsRt();

	//set pipeline state
	RHIContext->SetGraphicsPipilineState(GraphicsPSOs["BasePass"]);

	//prepare shader parameters
	RHIContext->PrepareShaderParameter();

	//pass sceen constant buffer
	RHIContext->SetSceneConstantBuffer(SceneConstantBuffer);

	//apply shadow map
	RHIContext->SetShadowMapSRV(ShadowMap->GetShadowMapResource());

	//Draw Rendering items in scene
	RHIContext->DrawRenderingMeshes(RenderingMeshes);

	//change back buffer state to present
	RHIContext->TransitionBackBufferStateToPresent();

	//excute command list
	RHIContext->EndDraw();

	//present backbuffer
	RHIContext->Present();

	//flush commands
	RHIContext->FlushCommandQueue();
}

void FRenderer::RenderDepth()
{
	RHIContext->SetViewport(*ShadowMap->GetViewport());
	RHIContext->SetScissor(0, 0, (long)ShadowMap->GetViewport()->Width, (long)ShadowMap->GetViewport()->Height);

	FRHIDepthResource* DepthResource = ShadowMap->GetShadowMapResource();
	IRHIResource* RHIResource = reinterpret_cast<IRHIResource*>(DepthResource);
	RHIContext->TransitionResource(RHIResource, ERHIResourceState::State_DepthRead, ERHIResourceState::State_DepthWrite);

	RHIContext->SetRenderTarget(nullptr, RHIResource);
	RHIContext->SetGraphicsPipilineState(GraphicsPSOs["DepthPass"]);

	//prepare shader parameters
	RHIContext->PrepareDepthShaderParameter();

	//pass sceen constant buffer
	RHIContext->SetSceneConstantBuffer(ShadowMap->GetSceneConstantBuffer());
	//Draw Rendering items in scene
	RHIContext->DrawRenderingMeshes(RenderingMeshes);
	RHIContext->TransitionResource(RHIResource, ERHIResourceState::State_DepthWrite, ERHIResourceState::State_DepthRead);

}

void FRenderer::UpdateConstantBuffer()
{
	//do with dynamic objects
	//for (int Index = 0 ; Index < RenderingMeshes.size(); ++Index)
	//{
	//	FMatrix World = Utilities::IdentityMatrix;

	//	// Update the constant buffer with the latest worldViewProj matrix.
	//	FObjectConstants ObjectConstant;
	//	ObjectConstant.ObjectWorld = World;
	//	ObjectConstant.Fresnel0 = FVector(0.04f, 0.04f, 0.04f);
	//	ObjectConstant.Shiness = 0.7f;
	//	ObjectConstant.AmbientLight = FVector(0.1f, 0.1f, 0.1f);

	//	RenderingMeshes[Index]->GetConstantBuffer()->CopyData(0, ObjectConstant);
	//}

}

void FRenderer::UpdateSceneConstantsBuffer(FSceneConstant* InSceneConstant)
{
	//rotate lighting
	FBoundSphere Bound;
	Bound.Center = FVector4D(0.0f, 0.0f, 0.0f, 0.0f);
	Bound.Radius = 500.0f;
	ShadowMap->AutomateRotateLight(Bound);

	//copy to upload buffer transposed???
	SceneConstant.ViewProj = InSceneConstant->ViewProj;
	SceneConstant.CamLocation = InSceneConstant->CamLocation;

	//pass light map light info to main pass , update lighting
	SceneConstant.LightDirection = ShadowMap->GetLightSceneConstant()->LightDirection;

	//set scene constants 
	FMatrix LightVP = ShadowMap->GetLightViewProj();
	// Transform NDC space [-1,+1]^2 to texture space [0,1]^2
	FMatrix T(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);

	SceneConstant.LightViewProj = LightVP * T;
	SceneConstant.LightViewProj = Utilities::MatrixTranspose(SceneConstant.LightViewProj);

	//update buffer
	SceneConstantBuffer->CopyData(0, SceneConstant);
}