#pragma once

#include "RHI/RHIContext.h"
#include "MeshActor.h"
#include "ShadowMap.h"

class FRenderer
{
public:
	FRenderer(){}
	~FRenderer(){}

	void CreateRHIContext(int InWidth, int Inheight);
	void DestroyRHIContext();

	void RenderObjects();

	void UpdateConstantBuffer();

	void UpdateSceneConstantsBuffer(FSceneConstant* InSceneConstant);

	void Resize(int InWidth, int InHeight);

	//
	void CreateRenderingItem(std::vector<std::unique_ptr<AMeshActor>>& Geometries);

protected:
private:

	IRHIContext* RHIContext;

	std::map<std::string, IRHIGraphicsPipelineState*> GraphicsPSOs;

	std::vector<IRHIRenderingMesh*> RenderingMeshes;

	//
	FViewport Viewport;

	//save camera info
	FSceneConstant SceneConstant;
	IRHIConstantBuffer<FSceneConstant>* SceneConstantBuffer;

	FShadowMap* ShadowMap;
};
