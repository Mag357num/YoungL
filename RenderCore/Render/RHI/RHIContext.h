#pragma once

#include <map>
#include <string>
#include <vector>
#include <intsafe.h>

using namespace std;

#include "RHIResource.h"
#include "RHIDepthResource.h"
#include "../../Utilities.h"

class IRHIGraphicsPipelineState
{
public:
	IRHIGraphicsPipelineState() {}
	virtual ~IRHIGraphicsPipelineState() {}
protected:
private:
};

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

class IRHIVertexBuffer
{
public:
	IRHIVertexBuffer() {}
	virtual ~IRHIVertexBuffer() {}
};

class IRHIIndexBuffer
{
public:
	IRHIIndexBuffer() {}
	virtual ~IRHIIndexBuffer() {}
};

template<typename T>
class IRHIConstantBuffer
{
public:
	IRHIConstantBuffer() {}
	virtual ~IRHIConstantBuffer() {}

	virtual void CopyData(int ElementIndex, const T& Data){}
protected:
};

class IRHIContext;
class IRHIRenderingMesh
{
public:
	IRHIRenderingMesh()
	{
		IsSkined = false;
	}

	virtual ~IRHIRenderingMesh(){

	}

	virtual void Release()
	{
		delete ConstantBuffer;
		ConstantBuffer = nullptr;

		delete VertexBuffer;
		VertexBuffer = nullptr;

		delete IndexBuffer;
		IndexBuffer = nullptr;

		delete BoneTransformsBuffer;
		BoneTransformsBuffer = nullptr;
	}

	virtual void BuildConstantBuffer(FObjectConstants* InObjConstants, IRHIContext* Context){}

	virtual void BuildSkinnedBoneTransBuffer(FBoneTransforms* InTransforms, IRHIContext* Context){}

	virtual void BuildVertexBuffer(std::vector<FVertex>& InVertices){}

	virtual void BuildVertexBuffer(std::vector<FSkinVertex>& InVertices){}
	
	virtual void BuildIndexBuffer(std::vector<uint16_t>& InIndices){}

	IRHIVertexBuffer* GetVertexBuffer(){return VertexBuffer;}
	IRHIIndexBuffer* GetIndexBuffer() { return IndexBuffer; }
	IRHIConstantBuffer<FObjectConstants>* GetConstantBuffer(){return ConstantBuffer;}
	
	IRHIConstantBuffer<FBoneTransforms>* GetBoneTransformsBuffer() { return BoneTransformsBuffer; }
	bool GetIsSkinned(){return IsSkined;}

	size_t GetIndexCount(){return IndexCount;}

protected:
	IRHIConstantBuffer<FObjectConstants>* ConstantBuffer;

	//used for skinned mesh
	bool IsSkined;
	IRHIConstantBuffer<FBoneTransforms>* BoneTransformsBuffer;

	IRHIVertexBuffer* VertexBuffer;
	IRHIIndexBuffer* IndexBuffer;

	int VertexStrideSize = 0;
	size_t VertexBufferSize = 0;
	size_t IndexBufferSize = 0;
	size_t IndexCount = 0;
	int InstanceCount = 0;
	int VertexBaseLocation = 0;
	int StartInstanceLocation = 0;
	//reserved for material
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

	virtual void SetViewport(const FViewport& Viewport){}
	virtual void SetScissor(long InX, long InY, long InWidth, long InHeight){}

	virtual IRHIShaderResource* CreateShaderResource(){ return nullptr; }
	virtual IRHIResource* CreateResource(){ return nullptr; }
	virtual IRHIResource* GetBackBufferResource(){return nullptr;}
	virtual IRHIVertexBuffer* CreateVertexBuffer(){ return nullptr; }
	virtual IRHIIndexBuffer* CreateIndexBuffer(){return nullptr;}
	virtual IRHIGraphicsPipelineState* CreateGraphicsPSO(){return nullptr;}
	virtual IRHIGraphicsPipelineState* CreateGraphicsDepthPSO() { return nullptr; }
	virtual IRHIGraphicsPipelineState* CreateSkinnedGraphicsPSO(){return nullptr;}

	virtual void TransitionResource(IRHIResource* InResource, ERHIResourceState StateBefore, ERHIResourceState StateAfter){}
	virtual void SetRenderTarget(IRHIResource* InColor, IRHIResource* InDepth){}
	
	//use back buffer as rt
	virtual void SetBackBufferAsRt(){}
	virtual void TransitionBackBufferStateToRT() {}
	virtual void TransitionBackBufferStateToPresent(){}

	//set graphic pso
	virtual void PrepareShaderParameter(){}
	virtual void PrepareDepthShaderParameter(){}
	virtual void PrepareSkinnedShaderParameter(){}

	virtual void SetGraphicsPipilineState(IRHIGraphicsPipelineState* InPSO){}


	virtual void SetSceneConstantBuffer(IRHIConstantBuffer<FSceneConstant>* InBuffer){}
	virtual void SetShaderResourceView(){}
	virtual void SetShadowMapSRV(FRHIDepthResource* InDepthResource){}

	virtual void SetPrimitiveTopology(){}

	virtual void DrawRenderingMeshes(std::unordered_map<std::string, IRHIRenderingMesh*>& Items){}
	virtual void DrawIndexedInstanced(){}

	virtual void FlushCommandQueue(){}

	virtual void Present(){}

	virtual IRHIRenderingMesh* CreateEmptyRenderingMesh(){return nullptr;}

	virtual IRHIConstantBuffer<FSceneConstant>* CreateSceneConstantBuffer(const FSceneConstant& SceneConstant){return nullptr;}

	virtual FRHIDepthResource* CreateShadowDepthResource(int InWidth, int InHeight, EPixelBufferFormat InFormat){ return nullptr; }
	virtual void CreateSrvDsvForDepthResource(FRHIDepthResource* InDepthResource) { }

protected:


};

