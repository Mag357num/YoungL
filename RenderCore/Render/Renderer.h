#pragma once

#include "RHI/RHIContext.h"
#include "../Game/StaticMeshActor.h"
#include "../Game/SkeletalMeshActor.h"
#include "ShadowMap.h"
#include "PostProcessing.h"

class FRenderer
{
public:
	FRenderer(){}
	~FRenderer(){}

	void CreateRHIContext(int InWidth, int Inheight);
	void DestroyRHIContext();

	void RenderScene();

	void UpdateConstantBuffer();

	void UpdateSceneConstantsBuffer(FSceneConstant* InSceneConstant);

	void UpdateSkinnedMeshBoneTransform(std::string ActorName, FBoneTransforms* InBoneTrans);

	void Resize(int InWidth, int InHeight);

	//
	void CreateRenderingItem(std::vector<std::unique_ptr<AStaticMeshActor>>& Geometries);
	void CreateRenderingItem(std::vector<std::unique_ptr<ASkeletalMeshActor>>& Geometries);

	void PostProcess();

protected:
private:
	void InitializeSceneConstant();

	void CreateBasePassPSO_Static();
	void CreateBasePassPSO_Skinned();
	void CreateDepthPassPSO();
	void CreatePresentPSO();
	void CreatePostProcessPSOs();
	

	void CreateSceneColor();

	void RenderDepth();
	void RenderSkinnedMesh();

	void DrawToBackBuffer();

	IRHIContext* RHIContext;

	std::map<std::string, IRHIGraphicsPipelineState*> GraphicsPSOs;

	std::unordered_map<std::string, IRHIRenderingMesh*> RenderingMeshes;
	std::unordered_map<std::string, IRHIRenderingMesh*> SkinnedRenderingMeshes;

	//
	FViewport Viewport;

	//save camera info
	FSceneConstant SceneConstant;
	IRHIConstantBuffer<FSceneConstant>* SceneConstantBuffer;

	FShadowMap* ShadowMap;

	//scene color
	FRHIColorResource* SceneColor;
	EPixelBufferFormat SceneColorFormat;

	//for postprocess
	bool ShouldRenderPostProcess;
	FPostProcessing* PostProcessing;
};
