#pragma once

#include <map>
#include <string>
#include <vector>
#include <intsafe.h>

using namespace std;

#include "RHIResource.h"
#include "RHIDepthResource.h"
#include "../Utilities.h"

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
	IRHIRenderingMesh(){}
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
	}

	virtual void BuildConstantBuffer(FObjectConstants* InObjConstants, IRHIContext* Context){}
	virtual void BuildVertexBuffer(std::vector<FVertex>& InVertices){}
	virtual void BuildIndexBuffer(std::vector<uint32_t>& InIndices){}

	IRHIVertexBuffer* GetVertexBuffer(){return VertexBuffer;}
	IRHIIndexBuffer* GetIndexBuffer() { return IndexBuffer; }
	IRHIConstantBuffer<FObjectConstants>* GetConstantBuffer(){return ConstantBuffer;}
	size_t GetIndexCount(){return IndexCount;}

protected:
	IRHIConstantBuffer<FObjectConstants>* ConstantBuffer;
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
	FViewport(float InX, float InY, float InWidth, float InHeight)
		:X(InX),
		Y(InY),
		Width(InWidth),
		Height(InHeight)
	{

	}

	FViewport(){}

	float X;
	float Y;
	float Width;
	float Height;

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

	virtual void BeginDraw(IRHIGraphicsPipelineState* InPSO, const wchar_t* Label){}
	virtual void EndDraw() {}


	virtual void SetViewport(const FViewport& Viewport){}
	virtual void SetScissor(long InX, long InY, long InWidth, long InHeight){}

	virtual IRHIShaderResource* CreateShaderResource(){ return nullptr; }
	virtual IRHIResource* CreateResource(){ return nullptr; }
	virtual IRHIResource* GetBackBufferResource(){return nullptr;}
	virtual IRHIVertexBuffer* CreateVertexBuffer(){ return nullptr; }
	virtual IRHIIndexBuffer* CreateIndexBuffer(){return nullptr;}
	virtual IRHIGraphicsPipelineState* CreateGraphicsPSO(){return nullptr;}

	virtual void TransitionResource(IRHIResource* InResource, ERHIResourceState StateBefore, ERHIResourceState StateAfter){}
	virtual void SetRenderTarget(IRHIResource* InResource){}
	
	//use back buffer as rt
	virtual void SetBackBufferAsRt(){}
	virtual void TransitionBackBufferStateToRT() {}
	virtual void TransitionBackBufferStateToPresent(){}

	//set graphic pso
	virtual void PrepareShaderParameter(){}
	virtual void SetGraphicsPipeline(IRHIGraphicsPipelineState* InPSO){}

	virtual void SetSceneConstantBuffer(IRHIConstantBuffer<FSceneConstant>* InBuffer){}
	virtual void SetShaderResourceView(){}

	virtual void SetPrimitiveTopology(){}

	virtual void DrawRenderingMeshes(std::vector<IRHIRenderingMesh*>& Items){}
	virtual void DrawIndexedInstanced(){}

	virtual void FlushCommandQueue(){}

	virtual void Present(){}

	virtual IRHIRenderingMesh* CreateEmptyRenderingMesh(){return nullptr;}

	virtual IRHIConstantBuffer<FSceneConstant>* CreateSceneConstantBuffer(const FSceneConstant& SceneConstant){return nullptr;}

	virtual FRHIDepthResource* CreateShadowDepthResource(int INWidth, int InHeight, EPixelBufferFormat InFormat){ return nullptr; }

protected:


};

