#pragma once

#include <map>
#include <string>
#include <vector>
#include <intsafe.h>

using namespace std;

#include "RHIResource.h"
#include "RHIDepthResource.h"
#include "RHIColorResource.h"
#include "../../Utilities.h"
#include "RHIGraphicsPipelineState.h"
#include "RHIConstantBuffer.h"
#include "RHIRenderingMesh.h"

class IRHIShaderResource
{
public:
	IRHIShaderResource() {}
	virtual ~IRHIShaderResource() {
		delete Handle;
		Handle = nullptr;
	}
protected:
	IRHIResourceHandle* Handle;
};

struct FViewport
{
	FViewport(int InX, int InY, int InWidth, int InHeight)
		:X(InX),
		Y(InY),
		Width(InWidth),
		Height(InHeight)
	{

	}

	FViewport(){}

	int X;
	int Y;
	int Width;
	int Height;

	float MaxDepth;
	float MinDepth;
};

class IRHIContext
{
public:
	IRHIContext(){}
	virtual ~IRHIContext(){}

	virtual void InitializeRHI(int InWidth, int InHeight){}

	virtual void Resize(int InWidth, int InHeight){}

	virtual void BeginDraw(const wchar_t* Label){}
	virtual void EndDraw() {}

	virtual void BeginEvent(const wchar_t* Label){}
	virtual void EndEvent(){}

	virtual void SetViewport(const FViewport& Viewport){}
	virtual void SetScissor(long InX, long InY, long InWidth, long InHeight){}

	virtual IRHIShaderResource* CreateShaderResource(){ return nullptr; }
	virtual IRHIResource* CreateResource(){ return nullptr; }
	virtual IRHIResource* GetBackBufferResource(){return nullptr;}
	//virtual IRHIVertexBuffer* CreateVertexBuffer(){ return nullptr; }
	//virtual IRHIIndexBuffer* CreateIndexBuffer(){return nullptr;}


	virtual IRHIGraphicsPipelineState* CreateGraphicsPSO(){return nullptr;}
	virtual IRHIGraphicsPipelineState* CreateGraphicsDepthPSO() { return nullptr; }
	virtual IRHIGraphicsPipelineState* CreateSkinnedGraphicsPSO(){return nullptr;}
	virtual IRHIGraphicsPipelineState* CreatePresentPipelineState(){return nullptr;}

	virtual void TransitionResource(IRHIResource* InResource, ERHIResourceState StateBefore, ERHIResourceState StateAfter){}
	virtual void SetRenderTarget(IRHIResource* InColor, IRHIResource* InDepth){}
	virtual void SetColorTarget(IRHIResource* InColor){}
	
	//use back buffer as rt
	virtual void SetBackBufferAsRt(){}
	virtual void TransitionBackBufferStateToRT() {}
	virtual void TransitionBackBufferStateToPresent(){}

	//set graphic pso
	virtual void PrepareShaderParameter(){}
	virtual void PrepareDepthShaderParameter(){}
	virtual void PrepareSkinnedShaderParameter(){}
	virtual void PreparePresentShaderParameter(){}

	virtual void SetGraphicsPipilineState(IRHIGraphicsPipelineState* InPSO){}


	virtual void SetSceneConstantBuffer(IRHIConstantBuffer<FSceneConstant>* InBuffer){}
	virtual void SetShaderResourceView(){}
	virtual void SetShadowMapSRV(FRHIDepthResource* InDepthResource){}
	virtual void SetColorSRV(UINT ParaIndex, FRHIColorResource* InColorResource){};

	virtual void SetPrimitiveTopology(){}

	virtual void DrawRenderingMeshes(std::unordered_map<std::string, IRHIRenderingMesh*>& Items){}
	virtual void DrawIndexedInstanced(){}
	virtual void Draw(UINT VertexCount, UINT VertexStartOffset = 0){}

	virtual void FlushCommandQueue(){}

	virtual void Present(){}

	virtual IRHIRenderingMesh* CreateEmptyRenderingMesh(){return nullptr;}

	virtual IRHIConstantBuffer<FSceneConstant>* CreateSceneConstantBuffer(const FSceneConstant& SceneConstant){return nullptr;}

	virtual FRHIDepthResource* CreateShadowDepthResource(int InWidth, int InHeight, EPixelBufferFormat InFormat){ return nullptr; }
	virtual void CreateSrvDsvForDepthResource(FRHIDepthResource* InDepthResource) { }

	virtual FRHIColorResource* CreateColorResource(int InWidth, int InHeight, EPixelBufferFormat InFormat){ return nullptr; }
	virtual void CreateSrvRtvForColorResource(FRHIColorResource* InColorResource) { }

protected:


};

