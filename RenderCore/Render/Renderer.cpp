#include "pch.h"
#include "Renderer.h"
#include "RHI/D3D12/RHIContext_D3D12.h"
#include "RHI/RHIShaderParameter.h"
#include "RHI/RHIShaderInputElement.h"

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

	CreateBasePassPSO_Static();
	CreateBasePassPSO_Skinned();
	CreatePresentPSO();


	//initialize scene constant
	InitializeSceneConstant();

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
		SceneConstant.LightViewProj = FMath::MatrixTranspose(SceneConstant.LightViewProj);

		CreateDepthPassPSO();
	}

	//create Scene Color
	CreateSceneColor();

	//Create Scene Constant Buffer
	SceneConstantBuffer = RHIContext->CreateSceneConstantBuffer(SceneConstant);
	SceneConstantBuffer->SetRootParameterIndex(1);//0 for Object constants 

	//for postprocess
	ShouldRenderPostProcess = false;
	if (ShouldRenderPostProcess)
	{
		PostProcessing = new FPostProcessing();
		PostProcessing->InitRTs(RHIContext, InWidth, Inheight);
		CreatePostProcessPSOs();
	}

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

	for (auto It = RenderingMeshes.begin(); It != RenderingMeshes.end(); ++It)
	{
		It->second->Release();
		delete It->second;
	}
	if (!RenderingMeshes.empty())
	{
		printf("Empty Error!");
	}

	//release skinned rendering mesh
	for (auto It = SkinnedRenderingMeshes.begin(); It != SkinnedRenderingMeshes.end(); ++It)
	{
		It->second->Release();
		delete It->second;
	}
	if (!SkinnedRenderingMeshes.empty())
	{
		printf("Empty Error!");
	}

	if (InstanceRenderingMeshes.size() > 0)
	{
		for (auto It = InstanceRenderingMeshes.begin(); It != InstanceRenderingMeshes.end(); ++It)
		{
			It->second->Release();
			delete It->second;
		}

		if (!InstanceRenderingMeshes.empty())
		{
		}
	}
	
	//shadowmap
	if (ShadowMap)
	{
		delete ShadowMap;
		ShadowMap = nullptr;
	}

	if (SceneColor)
	{
		delete SceneColor;
		SceneColor = nullptr;
	}

	//for postprocess
	if (PostProcessing)
	{
		PostProcessing->DestroyRTs();
		delete PostProcessing;
		PostProcessing = nullptr;
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
		Viewport.Width = InWidth;
		Viewport.Height = InHeight;

		RHIContext->Resize(InWidth, InHeight);

		//todo: resize pp
	}
}

void FRenderer::CreateRenderingItem(std::vector<std::unique_ptr<AStaticMeshActor>>& StaticMeshActors)
{
	for (int Index = 0; Index < StaticMeshActors.size(); ++Index)
	{
		//create an empty rendering item
		IRHIRenderingMesh* Item = RHIContext->CreateEmptyRenderingMesh();

		Item->BuildConstantBuffer(StaticMeshActors[Index]->GetObjectConstants(), RHIContext);
		std::weak_ptr<UStaticMesh> StaticMesh = StaticMeshActors[Index]->GetStaticMesh();

		if (StaticMesh.lock()->GetHasValidRenderResource())
		{
			std::shared_ptr<IRHIVertexBuffer> VertexBuffer = StaticMesh.lock()->GetVertexBuffer();
			std::shared_ptr<IRHIIndexBuffer> IndexBuffer = StaticMesh.lock()->GetIndexBuffer();;

			Item->SetVertexBuffer(VertexBuffer);
			Item->SetIndexBuffer(IndexBuffer);
		}
		else
		{
			FGeometry<FVertex>* StaticGeo = StaticMesh.lock()->GetGeometry();
			std::shared_ptr<IRHIVertexBuffer> VertexBuffer = Item->BuildVertexBuffer(StaticGeo->GetVertices());
			std::shared_ptr<IRHIIndexBuffer> IndexBuffer = Item->BuildIndexBuffer(StaticGeo->GetIndices());

			StaticMesh.lock()->SetVertexBuffer(VertexBuffer);
			StaticMesh.lock()->SetIndexBuffer(IndexBuffer);
			StaticMesh.lock()->SetHasValidRenderResource();

			Item->SetVertexBuffer(VertexBuffer);
			Item->SetIndexBuffer(IndexBuffer);
		}
		

		RenderingMeshes[*StaticMeshActors[Index]->GetName()] = Item;
	}
}

void FRenderer::CreateRenderingItem(std::vector<std::unique_ptr<ASkeletalMeshActor>>& SkeletalMeshActors)
{
	for (int Index = 0; Index < SkeletalMeshActors.size(); ++Index)
	{
		//create an empty rendering item

		IRHIRenderingMesh* Item = RHIContext->CreateEmptyRenderingMesh();

		Item->BuildConstantBuffer(SkeletalMeshActors[Index]->GetObjectConstants(), RHIContext);
		std::weak_ptr<USkeletalMesh> SkeletalMesh = SkeletalMeshActors[Index]->GetSkeletalMesh();
		
		if (SkeletalMesh.lock()->GetHasValidRenderResource())
		{
			std::shared_ptr<IRHIVertexBuffer> VertexBuffer = SkeletalMesh.lock()->GetVertexBuffer();
			std::shared_ptr<IRHIIndexBuffer> IndexBuffer = SkeletalMesh.lock()->GetIndexBuffer();;

			Item->SetVertexBuffer(VertexBuffer);
			Item->SetIndexBuffer(IndexBuffer);
		}
		else
		{
			FGeometry<FSkinVertex>* SkeletalGeo = SkeletalMesh.lock()->GetGeometry();
			std::shared_ptr<IRHIVertexBuffer> VertexBuffer = Item->BuildVertexBuffer(SkeletalGeo->GetVertices());
			std::shared_ptr<IRHIIndexBuffer> IndexBuffer = Item->BuildIndexBuffer(SkeletalGeo->GetIndices());

			SkeletalMesh.lock()->SetVertexBuffer(VertexBuffer);
			SkeletalMesh.lock()->SetIndexBuffer(IndexBuffer);
			SkeletalMesh.lock()->SetHasValidRenderResource();

			Item->SetVertexBuffer(VertexBuffer);
			Item->SetIndexBuffer(IndexBuffer);
		}

		//todo: create BoneTransform Constant Buffer
		Item->BuildSkinnedBoneTransBuffer(SkeletalMeshActors[Index]->GetBoneTransfroms(), RHIContext);

		SkinnedRenderingMeshes[*SkeletalMeshActors[Index]->GetName()] = Item;
	}
}

void FRenderer::CreateRenderingItem(std::vector<std::unique_ptr<AInstancedStaticMeshActor>>& InstancedActors)
{
	for (int Index = 0; Index < InstancedActors.size(); ++Index)
	{
		//create an empty rendering item
		IRHIRenderingMesh* Item = RHIContext->CreateEmptyRenderingMesh();

		Item->BuildConstantBuffer(InstancedActors[Index]->GetObjectConstants(), RHIContext);
		std::weak_ptr<UStaticMesh> StaticMesh = InstancedActors[Index]->GetStaticMesh();

		if (StaticMesh.lock()->GetHasValidRenderResource())
		{
			std::shared_ptr<IRHIVertexBuffer> VertexBuffer = StaticMesh.lock()->GetVertexBuffer();
			std::shared_ptr<IRHIIndexBuffer> IndexBuffer = StaticMesh.lock()->GetIndexBuffer();;

			Item->SetVertexBuffer(VertexBuffer);
			Item->SetIndexBuffer(IndexBuffer);
		}
		else
		{
			FGeometry<FVertex>* StaticGeo = StaticMesh.lock()->GetGeometry();
			std::shared_ptr<IRHIVertexBuffer> VertexBuffer = Item->BuildVertexBuffer(StaticGeo->GetVertices());
			std::shared_ptr<IRHIIndexBuffer> IndexBuffer = Item->BuildIndexBuffer(StaticGeo->GetIndices());

			StaticMesh.lock()->SetVertexBuffer(VertexBuffer);
			StaticMesh.lock()->SetIndexBuffer(IndexBuffer);
			StaticMesh.lock()->SetHasValidRenderResource();

			Item->SetVertexBuffer(VertexBuffer);
			Item->SetIndexBuffer(IndexBuffer);
		}


		InstanceRenderingMeshes[*InstancedActors[Index]->GetName()] = Item;
	}
}

void FRenderer::PostProcess()
{
	if (!PostProcessing)
	{
		return;
	}

	PostProcessing->BloomSetUp(RHIContext, SceneColor, GraphicsPSOs["BloomSetUp"]);

	PostProcessing->BloomDown(RHIContext, GraphicsPSOs["BloomDown"], 0);
	PostProcessing->BloomDown(RHIContext, GraphicsPSOs["BloomDown"], 1);
	PostProcessing->BloomDown(RHIContext, GraphicsPSOs["BloomDown"], 2);
	PostProcessing->BloomDown(RHIContext, GraphicsPSOs["BloomDown"], 3);

	PostProcessing->BloomUp(RHIContext, GraphicsPSOs["BloomUp"], 0);
	PostProcessing->BloomUp(RHIContext, GraphicsPSOs["BloomUp"], 1);
	PostProcessing->BloomUp(RHIContext, GraphicsPSOs["BloomUp"], 2);

	PostProcessing->BloomSunMerge(RHIContext, GraphicsPSOs["BloomSunMerge"]);

	//PostProcessing->CombineLUTs(RHIContext, GraphicsPSOs["CombineLUTs"]);
	PostProcessing->ToneMap(RHIContext, SceneColor, GraphicsPSOs["ToneMap"]);
}


void FRenderer::RenderScene()
{
	//reset command list and command allocator here
	RHIContext->BeginDraw(L"BasePass");

	//render depth map first
	//for realtime shadow, prepass
	RenderDepth();

	RHIContext->SetViewport(Viewport);
	RHIContext->SetScissor(0, 0, (long)Viewport.Width, (long)Viewport.Height);

	//use scene color as render target
	RHIContext->TransitionResource(SceneColor, State_GenerateRead, State_RenderTarget);

	//use specified color target && default depth stencil target
	RHIContext->SetColorTarget(SceneColor);

	//set pipeline state
	RHIContext->SetGraphicsPipilineState(GraphicsPSOs["BasePass"]);

	//prepare shader parameters
	RHIContext->PrepareShaderParameter();

	//pass sceen constant buffer
	RHIContext->SetConstantBufferView(SceneConstantBuffer->GetRootParameterIndex(), SceneConstantBuffer);

	//apply shadow map
	RHIContext->SetShadowMapSRV(ShadowMap->GetShadowMapResource());

	//Draw Rendering items in scene
	RHIContext->DrawRenderingMeshes(RenderingMeshes);

	RenderSkinnedMesh();

	//change back buffer state to present
	RHIContext->TransitionResource(SceneColor, State_RenderTarget, State_GenerateRead);

	//todo: for postprocess
	if (PostProcessing)
	{
		PostProcess();
	}
	//draw scene color to back buffer; may present hdr using tonemap
	if (!PostProcessing)
	{
		//replaced by tone map
		DrawToBackBuffer();
	}
	

	//excute command list
	RHIContext->EndDraw();

	//present backbuffer
	RHIContext->Present();

	//flush commands
	RHIContext->FlushCommandQueue();
}

void FRenderer::RenderDepth()
{
	RHIContext->BeginEvent(L"Depth Pass");

	RHIContext->SetViewport(*ShadowMap->GetViewport());
	RHIContext->SetScissor(0, 0, (long)ShadowMap->GetViewport()->Width, (long)ShadowMap->GetViewport()->Height);

	FRHIDepthResource* DepthResource = ShadowMap->GetShadowMapResource();
	IRHIResource* RHIResource = reinterpret_cast<IRHIResource*>(DepthResource);
	RHIContext->TransitionResource(RHIResource, ERHIResourceState::State_DepthRead, ERHIResourceState::State_DepthWrite);

	RHIContext->SetRenderTarget(nullptr, RHIResource);
	RHIContext->SetGraphicsPipilineState(GraphicsPSOs["DepthPass"]);

	//pass sceen constant buffer
	RHIContext->SetConstantBufferView(ShadowMap->GetSceneConstantBuffer()->GetRootParameterIndex(), ShadowMap->GetSceneConstantBuffer());
	//Draw Rendering items in scene
	RHIContext->DrawRenderingMeshes(RenderingMeshes);


	//draw skinned mesh
	// 	   todo: draw skinned mesh in depth pass
	//RenderSkinnedMesh();

	RHIContext->TransitionResource(RHIResource, ERHIResourceState::State_DepthWrite, ERHIResourceState::State_DepthRead);

	RHIContext->EndEvent();
}

void FRenderer::RenderSkinnedMesh()
{
	RHIContext->BeginEvent(L"Skinned");

	//draw skined Mesh
	RHIContext->SetGraphicsPipilineState(GraphicsPSOs["SkinPass"]);
	RHIContext->SetConstantBufferView(SceneConstantBuffer->GetRootParameterIndex(), SceneConstantBuffer);
	RHIContext->SetShadowMapSRV(ShadowMap->GetShadowMapResource());
	RHIContext->DrawRenderingMeshes(SkinnedRenderingMeshes);

	RHIContext->EndEvent();
}

//todo: present LDR or HDR
void FRenderer::DrawToBackBuffer()
{
	RHIContext->BeginEvent(L"Present");

	RHIContext->SetViewport(Viewport);
	RHIContext->SetScissor(0, 0, (long)Viewport.Width, (long)Viewport.Height);

	//change back buffer state to rendertarget
	RHIContext->TransitionBackBufferStateToRT();

	//set backbuffer as rendertarget
	RHIContext->SetBackBufferAsRt();

	//set pipeline state
	RHIContext->SetGraphicsPipilineState(GraphicsPSOs["PresentPass"]);

	//prepare shader parameters
	RHIContext->PreparePresentShaderParameter();

	//set root constant
	RHIContext->SetGraphicRootConstant(1, 1280, 0);
	//set root constant
	RHIContext->SetGraphicRootConstant(1, 720, 1);

	RHIContext->SetColorSRV(0, SceneColor);
	RHIContext->Draw(3);

	//	//change back buffer state to present
	RHIContext->TransitionBackBufferStateToPresent();

	RHIContext->EndEvent();
}


void FRenderer::InitializeSceneConstant()
{
	float AspectRatio = 1.0f * Viewport.Width / Viewport.Height;
	FMatrix Proj = FMath::MatrixPerspectiveFovLH(0.25f * 3.1416f, AspectRatio, 1.0f, 1000.0f);

	//// Build the initial view matrix.
	FVector4D CamPos = FVector4D(500, 500, 100, 0.0f);
	FVector4D CamTarget = FVector4D(0, 0, 150, 0.0f);
	FVector4D CamUp = FVector4D(0.0f, 0.0f, 1.0f, 0.0f);

	FMatrix View = FMath::MatrixLookAtLH(CamPos, CamTarget, CamUp);

	SceneConstant.ViewProj = View * Proj;

	//copy to upload buffer transposed???
	SceneConstant.ViewProj = FMath::MatrixTranspose(SceneConstant.ViewProj);
	SceneConstant.CamLocation = FVector4D(CamPos.X, CamPos.Y, CamPos.Z, 0.0f);

	//create global directional lighting
	SceneConstant.LightStrength = FVector4D(1.0f, 1.0f, 1.0f, 0.0f);
	SceneConstant.LightDirection = FVector4D(-1.0f, -1.0f, -1.0f, 0.0f);
}


void FRenderer::UpdateConstantBuffer()
{

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
	SceneConstant.LightViewProj = FMath::MatrixTranspose(SceneConstant.LightViewProj);

	//update buffer
	SceneConstantBuffer->CopyData(0, SceneConstant);
}

void FRenderer::UpdateSkinnedMeshBoneTransform(std::string ActorName, FBoneTransforms* InBoneTrans)
{
	FBoneTransforms BufferData = *InBoneTrans;
	SkinnedRenderingMeshes[ActorName]->GetBoneTransformsBuffer()->CopyData(0, BufferData);
}

void FRenderer::CreateSceneColor()
{
	SceneColorFormat=EPixelBufferFormat::PixelFormat_R16G16B16A16_Float;

	SceneColor = RHIContext->CreateColorResource(Viewport.Width, Viewport.Height, SceneColorFormat);
	//create srv and rtv for color resource
	RHIContext->CreateSrvRtvForColorResource(SceneColor);
}

void FRenderer::CreateBasePassPSO_Static()
{
	IRHIGraphicsPipelineState* BasePassPSO_Static = RHIContext->CreateEmpltyGraphicsPSO();
	
	//for shader parameter
	{
		FRHIShaderParameter SlotPara0(ParaType_CBV, 0, 0, Visibility_All);
		BasePassPSO_Static->AddShaderParameter(&SlotPara0);

		FRHIShaderParameter SlotPara1(ParaType_CBV, 1, 0, Visibility_All);
		BasePassPSO_Static->AddShaderParameter(&SlotPara1);

		FParameterRange ParamRange(RangeType_SRV, 1, 0, 0);
		FRHIShaderParameter SlotPara2(ParaType_Range, 0, 0, Visibility_PS);
		SlotPara2.AddRangeTable(ParamRange);
		BasePassPSO_Static->AddShaderParameter(&SlotPara2);

	}

	{
		FRHIShaderInputElement InputElement0("POSITION", 0, PixelFormat_R32G32B32_Float, 0, 0, INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0);
		BasePassPSO_Static->AddShaderInputElement(&InputElement0);

		FRHIShaderInputElement InputElement1("NORMAL", 0, PixelFormat_R32G32B32_Float, 0, 12, INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0);
		BasePassPSO_Static->AddShaderInputElement(&InputElement1);

		FRHIShaderInputElement InputElement2("TEXCOORD", 0, PixelFormat_R32G32_Float, 0, 24, INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0);
		BasePassPSO_Static->AddShaderInputElement(&InputElement2);
	}


	BasePassPSO_Static->SetCorlorTargetFormat(PixelFormat_R16G16B16A16_Float);
	BasePassPSO_Static->SetDepthTargetFormat(PixelFormat_D24_UNORM_S8_UINT);

	IRHIShader* VS = new IRHIShader();
	VS->SetShaderType(ShaderType_VS);
	VS->SetShaderPath(L"BasePassVS");
	BasePassPSO_Static->SetVS(VS);

	IRHIShader* PS = new IRHIShader();
	PS->SetShaderType(ShaderType_PS);
	PS->SetShaderPath(L"BasePassPS");
	BasePassPSO_Static->SetPS(PS);

	BasePassPSO_Static->SetDepthEnable(TRUE);

	BasePassPSO_Static->CreateGraphicsPSOInternal();
	GraphicsPSOs.insert(std::make_pair("BasePass", BasePassPSO_Static));
}

void FRenderer::CreateBasePassPSO_Skinned()
{
	IRHIGraphicsPipelineState* BasePassPSO_Skinned = RHIContext->CreateEmpltyGraphicsPSO();

	//for shader parameter
	{
		FRHIShaderParameter SlotPara0(ParaType_CBV, 0, 0, Visibility_All);
		BasePassPSO_Skinned->AddShaderParameter(&SlotPara0);

		FRHIShaderParameter SlotPara1(ParaType_CBV, 1, 0, Visibility_All);
		BasePassPSO_Skinned->AddShaderParameter(&SlotPara1);

		FParameterRange ParamRange(RangeType_SRV, 1, 0, 0);
		FRHIShaderParameter SlotPara2(ParaType_Range, 0, 0, Visibility_PS);
		SlotPara2.AddRangeTable(ParamRange);
		BasePassPSO_Skinned->AddShaderParameter(&SlotPara2);

		FRHIShaderParameter SlotPara3(ParaType_CBV, 2, 0, Visibility_All);
		BasePassPSO_Skinned->AddShaderParameter(&SlotPara3);
	}

	{
		FRHIShaderInputElement InputElement0("POSITION", 0, PixelFormat_R32G32B32_Float, 0, 0, INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0);
		BasePassPSO_Skinned->AddShaderInputElement(&InputElement0);

		FRHIShaderInputElement InputElement1("NORMAL", 0, PixelFormat_R32G32B32_Float, 0, 12, INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0);
		BasePassPSO_Skinned->AddShaderInputElement(&InputElement1);

		FRHIShaderInputElement InputElement2("TEXCOORD", 0, PixelFormat_R32G32_Float, 0, 24, INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0);
		BasePassPSO_Skinned->AddShaderInputElement(&InputElement2);

		FRHIShaderInputElement InputElement3("TAGANT", 0, PixelFormat_R32G32B32_Float, 0, 32, INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0);
		BasePassPSO_Skinned->AddShaderInputElement(&InputElement3);

		FRHIShaderInputElement InputElement4("BONEINDEX", 0, PixelFormat_R32G32B32A32_UINT, 0, 44, INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0);
		BasePassPSO_Skinned->AddShaderInputElement(&InputElement4);

		FRHIShaderInputElement InputElement5("BONEWEIGHT", 0, PixelFormat_R32G32B32A32_Float, 0, 60, INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0);
		BasePassPSO_Skinned->AddShaderInputElement(&InputElement5);
	}


	BasePassPSO_Skinned->SetCorlorTargetFormat(PixelFormat_R16G16B16A16_Float);
	BasePassPSO_Skinned->SetDepthTargetFormat(PixelFormat_D24_UNORM_S8_UINT);

	IRHIShader* VS = new IRHIShader();
	VS->SetShaderType(ShaderType_VS);
	VS->SetShaderPath(L"SkinnedMeshVS");
	BasePassPSO_Skinned->SetVS(VS);

	IRHIShader* PS = new IRHIShader();
	PS->SetShaderType(ShaderType_PS);
	PS->SetShaderPath(L"SkinnedMeshPS");
	BasePassPSO_Skinned->SetPS(PS);

	BasePassPSO_Skinned->SetDepthEnable(TRUE);

	BasePassPSO_Skinned->CreateGraphicsPSOInternal();
	GraphicsPSOs.insert(std::make_pair("SkinPass", BasePassPSO_Skinned));
}

void FRenderer::CreateDepthPassPSO()
{
	IRHIGraphicsPipelineState* DepthPassPSO = RHIContext->CreateEmpltyGraphicsPSO();

	//for shader parameter
	{
		FRHIShaderParameter SlotPara0(ParaType_CBV, 0, 0, Visibility_All);
		DepthPassPSO->AddShaderParameter(&SlotPara0);

		FRHIShaderParameter SlotPara1(ParaType_CBV, 1, 0, Visibility_All);
		DepthPassPSO->AddShaderParameter(&SlotPara1);

	}

	{
		FRHIShaderInputElement InputElement0("POSITION", 0, PixelFormat_R32G32B32_Float, 0, 0, INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0);
		DepthPassPSO->AddShaderInputElement(&InputElement0);

		FRHIShaderInputElement InputElement1("NORMAL", 0, PixelFormat_R32G32B32_Float, 0, 12, INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0);
		DepthPassPSO->AddShaderInputElement(&InputElement1);

		FRHIShaderInputElement InputElement2("TEXCOORD", 0, PixelFormat_R32G32_Float, 0, 24, INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0);
		DepthPassPSO->AddShaderInputElement(&InputElement2);
	}


	DepthPassPSO->SetCorlorTargetFormat(PixelFormat_R16G16B16A16_Float);
	DepthPassPSO->SetDepthTargetFormat(PixelFormat_D24_UNORM_S8_UINT);

	IRHIShader* VS = new IRHIShader();
	VS->SetShaderType(ShaderType_VS);
	VS->SetShaderPath(L"DepthVS");
	DepthPassPSO->SetVS(VS);

	IRHIShader* PS = new IRHIShader();
	PS->SetShaderType(ShaderType_PS);
	PS->SetShaderPath(L"DepthPS");
	DepthPassPSO->SetPS(PS);

	DepthPassPSO->SetDepthEnable(TRUE);

	DepthPassPSO->CreateGraphicsPSOInternal();
	GraphicsPSOs.insert(std::make_pair("DepthPass", DepthPassPSO));
}

void FRenderer::CreatePresentPSO()
{
	//create present pass
	IRHIGraphicsPipelineState* PresentPSO = RHIContext->CreateEmpltyGraphicsPSO();

	FParameterRange ParamRange(RangeType_SRV, 1, 0, 0);
	FRHIShaderParameter ShaderParam(ParaType_Range, 0, 0, Visibility_PS);
	ShaderParam.AddRangeTable(ParamRange);
	PresentPSO->AddShaderParameter(&ShaderParam);

	FRHIShaderParameter ConstantPara(ParaType_Constant, 0, 0, Visibility_PS);
	ConstantPara.SetNum32BitValues(2);
	PresentPSO->AddShaderParameter(&ConstantPara);

	PresentPSO->SetCorlorTargetFormat(PixelFormat_R8G8B8A8_Unorm);

	FRHISamplerState SampleState(0, 0, Filter_MIN_MAG_LINEAR_MIP_POINT, ADDRESS_MODE_CLAMP, ADDRESS_MODE_CLAMP, ADDRESS_MODE_CLAMP);
	PresentPSO->AddSampleState(&SampleState);
	
	IRHIShader* VS = new IRHIShader();
	VS->SetShaderType(ShaderType_VS);
	VS->SetShaderPath(L"ScreenVS");
	PresentPSO->SetVS(VS);

	IRHIShader* PS = new IRHIShader();
	PS->SetShaderType(ShaderType_PS);
	PS->SetShaderPath(L"ScreenPS");
	PresentPSO->SetPS(PS);
	
	PresentPSO->CreateGraphicsPSOInternal();

	GraphicsPSOs.insert(std::make_pair("PresentPass", PresentPSO));
}

void FRenderer::CreatePostProcessPSOs()
{
	IRHIGraphicsPipelineState* BloomSetUpPSO = PostProcessing->CreateBloomSetUpPSO(RHIContext);
	GraphicsPSOs.insert(std::make_pair("BloomSetUp", BloomSetUpPSO));

	IRHIGraphicsPipelineState* BloomDownPSO = PostProcessing->CreateBloomDownPSO(RHIContext);
	GraphicsPSOs.insert(std::make_pair("BloomDown", BloomDownPSO));

	IRHIGraphicsPipelineState* BloomUpPSO = PostProcessing->CreateBloomUpPSO(RHIContext);
	GraphicsPSOs.insert(std::make_pair("BloomUp", BloomUpPSO));

	IRHIGraphicsPipelineState* BloomSunMergePSO = PostProcessing->CreateBloomSunMergePSO(RHIContext);
	GraphicsPSOs.insert(std::make_pair("BloomSunMerge", BloomSunMergePSO));

	IRHIGraphicsPipelineState* CombineLUTsPSO = PostProcessing->CreateCombineLUTsPSO(RHIContext);
	GraphicsPSOs.insert(std::make_pair("CombineLUTs", CombineLUTsPSO));

	IRHIGraphicsPipelineState* ToneMapPSO = PostProcessing->CreateToneMapPSO(RHIContext);
	GraphicsPSOs.insert(std::make_pair("ToneMap", ToneMapPSO));
}