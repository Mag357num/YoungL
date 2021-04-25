#pragma once

#include "RHI/RHIContext.h"
#include "../Game/MeshActor.h"
#include "../Game/SkinMeshActor.h"
#include "ShadowMap.h"

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
	void CreateRenderingItem(std::vector<std::unique_ptr<AMeshActor>>& Geometries);
	void CreateRenderingItem(std::vector<std::unique_ptr<ASkinMeshActor>>& Geometries);

protected:
private:
	
	void CreateSceneColor();

	void RenderDepth();
	void RenderSkinnedMesh();

	void PresentLDR();

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
};
