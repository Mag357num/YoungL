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
#include "RHIShader.h"
#include "Formats.h"


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
	IRHIContext(){
		BackBufferFormat = PixelFormat_R8G8B8A8_Unorm;
		SceneColorFormat = PixelFormat_R10G10B10A2_UNorm;
	}
	virtual ~IRHIContext(){}

	virtual void InitializeRHI(int InWidth, int InHeight){}

	virtual void Resize(int InWidth, int InHeight){}

	virtual void BeginDraw(const wchar_t* Label){}
	virtual void EndDraw() {}

	virtual void BeginEvent(const wchar_t* Label){}
	virtual void EndEvent(){}

	virtual void SetViewport(const FViewport& Viewport){}
	virtual void SetScissor(long InX, long InY, long InWidth, long InHeight){}

	virtual IRHIShader* CreateShaderResource(){ return nullptr; }
	virtual IRHIResource* CreateResource(){ return nullptr; }
	virtual IRHIResource* GetBackBufferResource(){return nullptr;}

	virtual IRHIGraphicsPipelineState* CreateEmpltyGraphicsPSO(){return nullptr;}


	virtual void TransitionResource(IRHIResource* InResource, ERHIResourceState StateBefore, ERHIResourceState StateAfter){}
	virtual void SetRenderTarget(IRHIResource* InColor, IRHIResource* InDepth){}
	virtual void SetColorTarget(IRHIResource* InColor){}
	
	//use back buffer as rt
	virtual void SetBackBufferAsRt(){}
	virtual void TransitionBackBufferStateToRT() {}
	virtual void TransitionBackBufferStateToPresent(){}

	//set graphic pso
	virtual void PrepareShaderParameter(){}
	virtual void PreparePresentShaderParameter(){}

	virtual void SetGraphicsPipilineState(IRHIGraphicsPipelineState* InPSO){}

	virtual void SetGraphicRootConstant(UINT SlotParaIndex, UINT SrcData, UINT DestOffsetIn32BitValues){}

	//for draw call info
	virtual void SetVertexBuffer(UINT StartSlot, UINT NumViews, IRHIVertexBuffer* VertexBuffer){}
	virtual void SetInstanceVertexBuffer(UINT StartSlot, IRHIVertexBuffer* VertexBuffer, IRHIVertexBuffer* InstanceBuffer){}
	virtual void SetIndexBuffer(IRHIIndexBuffer* IndexBuffer){}
	virtual void SetPrimitiveTopology(EPrimitiveTopology Topology){}

	//for constant buffer view
	virtual void SetSceneConstantBufferView(UINT SlotParaIndex, IRHIConstantBuffer<FSceneConstant>* InBuffer){}
	virtual void SetObjectConstantBufferView(UINT SlotParaIndex, IRHIConstantBuffer<FObjectConstants>* InBuffer){}
	virtual void SetBoneTransformConstantBufferView(UINT SlotParaIndex, IRHIConstantBuffer<FBoneTransforms>* InBuffer){}
	
	virtual void SetShaderResourceView(){}
	virtual void SetDepthAsSRV(UINT ParaIndex, FRHIDepthResource* InDepthResource){}
	virtual void SetColorSRV(UINT ParaIndex, FRHIColorResource* InColorResource){};

	virtual void SetPrimitiveTopology(){}

	virtual void DrawIndexedInstanced(UINT IndexCountPerInstance, UINT IndexCount, 
								UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation){}

	virtual void Draw(UINT VertexCount, UINT VertexStartOffset = 0){}

	virtual void FlushCommandQueue(){}

	virtual void Present(){}

	virtual IRHIRenderingMesh* CreateEmptyRenderingMesh(){return nullptr;}

	virtual IRHIConstantBuffer<FSceneConstant>* CreateSceneConstantBuffer(const FSceneConstant& SceneConstant){return nullptr;}

	virtual FRHIDepthResource* CreateDepthResource(int InWidth, int InHeight, EPixelBufferFormat InFormat){ return nullptr; }
	virtual void CreateSrvDsvForDepthResource(FRHIDepthResource* InDepthResource) { }

	virtual FRHIColorResource* CreateColorResource(int InWidth, int InHeight, EPixelBufferFormat InFormat, bool NeedUpload = false){ return nullptr; }
	virtual void CreateSrvRtvForColorResource(FRHIColorResource* InColorResource) { }
	virtual void CreateSrvForColorResource(FRHIColorResource* InColorResource) { }
	virtual void CopyTextureDataToResource(std::vector<FColor>& Colors, UINT Width, UINT Height, FRHIColorResource* ColorResource){}

protected:
	EPixelBufferFormat BackBufferFormat;
	EPixelBufferFormat SceneColorFormat;

};

