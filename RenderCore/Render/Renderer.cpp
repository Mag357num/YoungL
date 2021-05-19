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

	//create render resource manager
	ResourceManager = new FRenderResourceManager();

	//create PSO Manager
	PSOManager = new FPSOManager();

	PSOManager->CreateBasePassPSO_Static(RHIContext, "BasePass");
	PSOManager->CreateBasePassPSO_Skinned(RHIContext, "SkinPass");
	PSOManager->CreateInstantcedPassPSO(RHIContext, "InstancePass");
	PSOManager->CreatePresentPSO(RHIContext, "PresentPass");

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

		PSOManager->CreateDepthPassPSO(RHIContext, "DepthPass");
	}

	//create Scene Color
	CreateSceneColor();

	//Create Scene Constant Buffer
	SceneConstantBuffer = RHIContext->CreateSceneConstantBuffer(SceneConstant);
	SceneConstantBuffer->SetRootParameterIndex(1);//0 for Object constants 

	//on & offs
	ShouldRenderShadow = true;
	ShouldRenderStatic = true;
	ShouldRenderSkeletal = true;
	ShouldRenderInstanced = false;
	ShouldAutoRotateLight = false;

	//for postprocess
	ShouldRenderPostProcess = true;


	if (ShouldRenderPostProcess)
	{
		PostProcessing = new FPostProcessing();
		PostProcessing->InitRTs(RHIContext, InWidth, Inheight);
		PSOManager->CreatePostProcessPSOs(RHIContext, PostProcessing);
	}

	//create gpu driven
	GPUDriven = new FGPUDriven();
	GPUDriven->InitFrustumCull(RHIContext, PSOManager);

}

void FRenderer::DestroyRHIContext()
{
	if (SceneConstantBuffer)
	{
		delete SceneConstantBuffer;
		SceneConstantBuffer = nullptr;
	}

	if (RenderingMeshes.size() > 0)
	{
		for (auto It = RenderingMeshes.begin(); It != RenderingMeshes.end(); ++It)
		{
			It->second->Release();
			delete It->second;
		}

		RenderingMeshes.clear();
		if (!RenderingMeshes.empty())
		{
			Utilities::Print("Empty RenderingMeshes Error! \n");
		}
	}
	

	//release skinned rendering mesh
	if (SkinnedRenderingMeshes.size() > 0)
	{
		for (auto It = SkinnedRenderingMeshes.begin(); It != SkinnedRenderingMeshes.end(); ++It)
		{
			It->second->Release();
			delete It->second;
		}

		SkinnedRenderingMeshes.clear();
		if (!SkinnedRenderingMeshes.empty())
		{
			Utilities::Print("Empty SkinnedRenderingMeshes Error! \n");
		}
	}
	

	if (InstanceRenderingMeshes.size() > 0)
	{
		for (auto It = InstanceRenderingMeshes.begin(); It != InstanceRenderingMeshes.end(); ++It)
		{
			It->second->Release();
			delete It->second;
		}

		InstanceRenderingMeshes.clear();
		if (!InstanceRenderingMeshes.empty())
		{
			Utilities::Print("Empty InstanceRenderingMeshes Error! \n");
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


	//destroy render resource manager
	if (ResourceManager)
	{
		delete ResourceManager;
		ResourceManager = nullptr;
	}

	if (PSOManager)
	{
		delete PSOManager;
		PSOManager = nullptr;
	}

	if (GPUDriven)
	{
		delete GPUDriven;
		GPUDriven = nullptr;
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

		FMeshRenderResource* RenderResouce = ResourceManager->CheckHasValidRenderResource(StaticMesh.lock()->GetObjectName());

		if (RenderResouce)
		{
			std::shared_ptr<IRHIVertexBuffer> VertexBuffer = RenderResouce->GetVertexBuffer();
			std::shared_ptr<IRHIIndexBuffer> IndexBuffer = RenderResouce->GetIndexBuffer();;

			Item->SetVertexBuffer(VertexBuffer);
			Item->SetIndexBuffer(IndexBuffer);

			Item->SetVertexStrideSize(RenderResouce->GetVertexStrideSize());
			Item->SetVertexBufferSize(RenderResouce->GetVertexBufferSize());
			Item->SetIndexBufferSize(RenderResouce->GetIndexBufferSize());
			Item->SetIndexCount(RenderResouce->GetIndexCount());
		}
		else
		{
			RenderResouce = new FMeshRenderResource();
			FGeometry<FVertex>* StaticGeo = StaticMesh.lock()->GetGeometry();
			std::shared_ptr<IRHIVertexBuffer> VertexBuffer = Item->BuildVertexBuffer(StaticGeo->GetVertices());
			std::shared_ptr<IRHIIndexBuffer> IndexBuffer = Item->BuildIndexBuffer(StaticGeo->GetIndices());

			RenderResouce->SetVertexBuffer(VertexBuffer);
			RenderResouce->SetIndexBuffer(IndexBuffer);

			RenderResouce->SetVertexStrideSize(Item->GetVertexStrideSize());
			RenderResouce->SetVertexBufferSize(Item->GetVertexBufferSize());
			RenderResouce->SetIndexBufferSize(Item->GetIndexBufferSize());
			RenderResouce->SetIndexCount(Item->GetIndexCount());

			Item->SetVertexBuffer(VertexBuffer);
			Item->SetIndexBuffer(IndexBuffer);

			ResourceManager->CacheMeshRenderResource(StaticMesh.lock()->GetObjectName(), RenderResouce);
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
		
		FMeshRenderResource* RenderResouce = ResourceManager->CheckHasValidRenderResource(SkeletalMesh.lock()->GetObjectName());

		if (RenderResouce)
		{
			std::shared_ptr<IRHIVertexBuffer> VertexBuffer = RenderResouce->GetVertexBuffer();
			std::shared_ptr<IRHIIndexBuffer> IndexBuffer = RenderResouce->GetIndexBuffer();;

			Item->SetVertexBuffer(VertexBuffer);
			Item->SetIndexBuffer(IndexBuffer);

			Item->SetVertexStrideSize(RenderResouce->GetVertexStrideSize());
			Item->SetVertexBufferSize(RenderResouce->GetVertexBufferSize());
			Item->SetIndexBufferSize(RenderResouce->GetIndexBufferSize());
			Item->SetIndexCount(RenderResouce->GetIndexCount());
		}
		else
		{
			RenderResouce = new FMeshRenderResource();
			FGeometry<FSkinVertex>* SkeletalGeo = SkeletalMesh.lock()->GetGeometry();
			std::shared_ptr<IRHIVertexBuffer> VertexBuffer = Item->BuildVertexBuffer(SkeletalGeo->GetVertices());
			std::shared_ptr<IRHIIndexBuffer> IndexBuffer = Item->BuildIndexBuffer(SkeletalGeo->GetIndices());

			RenderResouce->SetVertexBuffer(VertexBuffer);
			RenderResouce->SetIndexBuffer(IndexBuffer);

			RenderResouce->SetVertexStrideSize(Item->GetVertexStrideSize());
			RenderResouce->SetVertexBufferSize(Item->GetVertexBufferSize());
			RenderResouce->SetIndexBufferSize(Item->GetIndexBufferSize());
			RenderResouce->SetIndexCount(Item->GetIndexCount());

			Item->SetVertexBuffer(VertexBuffer);
			Item->SetIndexBuffer(IndexBuffer);

			ResourceManager->CacheMeshRenderResource(SkeletalMesh.lock()->GetObjectName(), RenderResouce);
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

		FMeshRenderResource* RenderResouce = ResourceManager->CheckHasValidRenderResource(StaticMesh.lock()->GetObjectName());

		if (RenderResouce)
		{
			std::shared_ptr<IRHIVertexBuffer> VertexBuffer = RenderResouce->GetVertexBuffer();
			std::shared_ptr<IRHIIndexBuffer> IndexBuffer = RenderResouce->GetIndexBuffer();;

			Item->SetVertexBuffer(VertexBuffer);
			Item->SetIndexBuffer(IndexBuffer);

			Item->SetVertexStrideSize(RenderResouce->GetVertexStrideSize());
			Item->SetVertexBufferSize(RenderResouce->GetVertexBufferSize());
			Item->SetIndexBufferSize(RenderResouce->GetIndexBufferSize());
			Item->SetIndexCount(RenderResouce->GetIndexCount());
		}
		else
		{
			RenderResouce = new FMeshRenderResource();
			FGeometry<FVertex>* StaticGeo = StaticMesh.lock()->GetGeometry();
			std::shared_ptr<IRHIVertexBuffer> VertexBuffer = Item->BuildVertexBuffer(StaticGeo->GetVertices());
			std::shared_ptr<IRHIIndexBuffer> IndexBuffer = Item->BuildIndexBuffer(StaticGeo->GetIndices());

			RenderResouce->SetVertexBuffer(VertexBuffer);
			RenderResouce->SetIndexBuffer(IndexBuffer);

			RenderResouce->SetVertexStrideSize(Item->GetVertexStrideSize());
			RenderResouce->SetVertexBufferSize(Item->GetVertexBufferSize());
			RenderResouce->SetIndexBufferSize(Item->GetIndexBufferSize());
			RenderResouce->SetIndexCount(Item->GetIndexCount());

			Item->SetVertexBuffer(VertexBuffer);
			Item->SetIndexBuffer(IndexBuffer);

			ResourceManager->CacheMeshRenderResource(StaticMesh.lock()->GetObjectName(), RenderResouce);
		}

		std::vector<FInstanceData> InstanceDatas = InstancedActors[Index]->CalcInstanceDatas();
		std::shared_ptr<IRHIVertexBuffer> InstanceBuffer = Item->BuildInstanceBuffer(InstanceDatas);
		Item->SetInstanceBuffer(InstanceBuffer);
		
		Item->SetInstanceCount(InstancedActors[Index]->GetInstanceCount());

		InstanceRenderingMeshes[*InstancedActors[Index]->GetName()] = Item;
	}
}

void FRenderer::PostProcess()
{
	if (!PostProcessing)
	{
		return;
	}

	PostProcessing->BloomSetUp(RHIContext, SceneColor, PSOManager->GetGraphicsPSO("BloomSetUp"));

	PostProcessing->BloomDown(RHIContext, PSOManager->GetGraphicsPSO("BloomDown"), 0);
	PostProcessing->BloomDown(RHIContext, PSOManager->GetGraphicsPSO("BloomDown"), 1);
	PostProcessing->BloomDown(RHIContext, PSOManager->GetGraphicsPSO("BloomDown"), 2);
	PostProcessing->BloomDown(RHIContext, PSOManager->GetGraphicsPSO("BloomDown"), 3);

	PostProcessing->BloomUp(RHIContext, PSOManager->GetGraphicsPSO("BloomUp"), 0);
	PostProcessing->BloomUp(RHIContext, PSOManager->GetGraphicsPSO("BloomUp"), 1);
	PostProcessing->BloomUp(RHIContext, PSOManager->GetGraphicsPSO("BloomUp"), 2);

	PostProcessing->BloomSunMerge(RHIContext, PSOManager->GetGraphicsPSO("BloomSunMerge"));

	//PostProcessing->CombineLUTs(RHIContext, GraphicsPSOs["CombineLUTs"]);
	PostProcessing->ToneMap(RHIContext, SceneColor, PSOManager->GetGraphicsPSO("ToneMap"));
}


void FRenderer::RenderScene()
{
	//reset command list and command allocator here
	RHIContext->BeginDraw(L"BasePass");

	//render depth map first
	//for realtime shadow, prepass
	if (ShouldRenderShadow)
	{
		RenderDepth();
	}
	

	RHIContext->SetViewport(Viewport);
	RHIContext->SetScissor(0, 0, (long)Viewport.Width, (long)Viewport.Height);
	RHIContext->SetPrimitiveTopology(PrimitiveTopology_TRIANGLELIST);

	//use scene color as render target
	RHIContext->TransitionResource(SceneColor, State_GenerateRead, State_RenderTarget);

	//use specified color target && default depth stencil target
	RHIContext->SetColorTarget(SceneColor);

	//set pipeline state
	RHIContext->SetGraphicsPipilineState(PSOManager->GetGraphicsPSO("BasePass"));

	//prepare shader parameters
	RHIContext->PrepareShaderParameter();

	//pass sceen constant buffer
	RHIContext->SetSceneConstantBufferView(SceneConstantBuffer->GetRootParameterIndex(), SceneConstantBuffer);

	//apply shadow map
	if (ShouldRenderShadow)
	{
		RHIContext->SetDepthAsSRV(2, ShadowMap->GetShadowMapResource());
	}

	if (ShouldRenderStatic)
	{
		//Draw Rendering items in scene
		DrawRenderingMeshes(RenderingMeshes);
	}
	
	if (ShouldRenderSkeletal)
	{
		RenderSkinnedMesh();
	}
	

	if (ShouldRenderInstanced)
	{
		//draw instanced
		RenderInstancedMesh();
	}

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
	RHIContext->SetGraphicsPipilineState(PSOManager->GetGraphicsPSO("DepthPass"));

	//pass sceen constant buffer
	RHIContext->SetSceneConstantBufferView(ShadowMap->GetSceneConstantBuffer()->GetRootParameterIndex(), ShadowMap->GetSceneConstantBuffer());
	//Draw Rendering items in scene
	DrawRenderingMeshes(RenderingMeshes);


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
	RHIContext->SetGraphicsPipilineState(PSOManager->GetGraphicsPSO("SkinPass"));
	RHIContext->SetSceneConstantBufferView(SceneConstantBuffer->GetRootParameterIndex(), SceneConstantBuffer);
	RHIContext->SetDepthAsSRV(2, ShadowMap->GetShadowMapResource());
	DrawRenderingMeshes(SkinnedRenderingMeshes);

	RHIContext->EndEvent();
}

void FRenderer::RenderInstancedMesh()
{
	RHIContext->BeginEvent(L"Instance");

	//draw skined Mesh
	RHIContext->SetGraphicsPipilineState(PSOManager->GetGraphicsPSO("InstancePass"));
	RHIContext->SetSceneConstantBufferView(SceneConstantBuffer->GetRootParameterIndex(), SceneConstantBuffer);
	RHIContext->SetDepthAsSRV(2, ShadowMap->GetShadowMapResource());
	DrawRenderingMeshes(InstanceRenderingMeshes);

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
	RHIContext->SetGraphicsPipilineState(PSOManager->GetGraphicsPSO("PresentPass"));

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

void FRenderer::DrawRenderingMeshes(std::unordered_map<std::string, IRHIRenderingMesh*>& Items)
{
	for (auto It = Items.begin(); It != Items.end(); ++It)
	{
		
		if (It->second->GetIsInstance())
		{
			RHIContext->SetInstanceVertexBuffer(0, It->second->GetVertexBuffer(), It->second->GetInstanceBuffer());
		}
		else
		{
			RHIContext->SetVertexBuffer(0, 1, It->second->GetVertexBuffer());
		}
		
		RHIContext->SetIndexBuffer(It->second->GetIndexBuffer());
		RHIContext->SetPrimitiveTopology(PrimitiveTopology_TRIANGLELIST);

		IRHIConstantBuffer<FObjectConstants>* ConstantBuffer = It->second->GetConstantBuffer();
		RHIContext->SetObjectConstantBufferView(ConstantBuffer->GetRootParameterIndex(), ConstantBuffer);

		if (It->second->GetIsSkinned())
		{
			IRHIConstantBuffer<FBoneTransforms>* BoneTransformsBuffer = It->second->GetBoneTransformsBuffer();
			RHIContext->SetBoneTransformConstantBufferView(BoneTransformsBuffer->GetRootParameterIndex(), BoneTransformsBuffer);
		}

		RHIContext->DrawIndexedInstanced((UINT)It->second->GetIndexCount(), (UINT)It->second->GetInstanceCount(), 0, 0, 0);


	}
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
	if (ShouldAutoRotateLight)
	{
		FBoundSphere Bound;
		Bound.Center = FVector4D(0.0f, 0.0f, 0.0f, 0.0f);
		Bound.Radius = 500.0f;
		ShadowMap->AutomateRotateLight(Bound);
	}
	

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
	std::unordered_map<std::string, IRHIRenderingMesh*>::iterator Iter;
	Iter = SkinnedRenderingMeshes.find(ActorName);
	if (Iter != SkinnedRenderingMeshes.end())
	{
		FBoneTransforms BufferData = *InBoneTrans;
		Iter->second->GetBoneTransformsBuffer()->CopyData(0, BufferData);
	}

}

void FRenderer::CreateSceneColor()
{
	SceneColorFormat=EPixelBufferFormat::PixelFormat_R16G16B16A16_Float;

	FColorResourceDesc ColorDesc;
	ColorDesc.Width = Viewport.Width;
	ColorDesc.Height = Viewport.Height;
	ColorDesc.ResourceFlag = Resource_Allow_Render_Target;
	ColorDesc.ResourceState = State_GenerateRead;
	ColorDesc.Format = SceneColorFormat;

	SceneColor = RHIContext->CreateColorResource(ColorDesc);
	//create srv and rtv for color resource
	RHIContext->CreateSrvRtvForColorResource(SceneColor);
}