#pragma once
//#include "RHIContext.h"
#include "RHIIndexBuffer.h"
#include "RHIVertexBuffer.h"
#include "RHIConstantBuffer.h"

class IRHIContext;
class IRHIRenderingMesh
{
public:
	IRHIRenderingMesh()
	{
		IsSkined = false;
		IsInstance = false;
	}

	virtual ~IRHIRenderingMesh() {

	}

	virtual void Release()
	{
		delete ConstantBuffer;
		ConstantBuffer = nullptr;

		//delete VertexBuffer;
		//VertexBuffer = nullptr;

		//delete IndexBuffer;
		//IndexBuffer = nullptr;

		delete BoneTransformsBuffer;
		BoneTransformsBuffer = nullptr;
	}

	virtual void BuildConstantBuffer(FObjectConstants* InObjConstants, IRHIContext* Context) {}
	virtual void BuildInstanceBuffer(std::vector<FInstanceData>& InstanceDatas, IRHIContext* Context){}
	virtual void BuildSkinnedBoneTransBuffer(FBoneTransforms* InTransforms, IRHIContext* Context) {}

	virtual std::shared_ptr<FRHIColorResource> BuildInstanceBuffer(std::shared_ptr<UTexture> InstanceTexture, IRHIContext* Context){return nullptr;}

	virtual std::shared_ptr<IRHIVertexBuffer> BuildVertexBuffer(std::vector<FVertex>& InVertices) { return nullptr; }
	virtual std::shared_ptr<IRHIVertexBuffer> BuildVertexBuffer(std::vector<FSkinVertex>& InVertices) {return nullptr;}
	virtual std::shared_ptr<IRHIIndexBuffer> BuildIndexBuffer(std::vector<uint16_t>& InIndices) { return nullptr; }

	IRHIVertexBuffer* GetVertexBuffer() { return VertexBuffer.lock().get(); }
	IRHIIndexBuffer* GetIndexBuffer() { return IndexBuffer.lock().get(); }
	void SetVertexBuffer(std::shared_ptr<IRHIVertexBuffer> InVertexBuffer){ VertexBuffer = InVertexBuffer;}
	void SetIndexBuffer(std::shared_ptr<IRHIIndexBuffer> InIndexBuffer){IndexBuffer = InIndexBuffer;}


	IRHIConstantBuffer<FObjectConstants>* GetConstantBuffer() { return ConstantBuffer; }

	IRHIConstantBuffer<FBoneTransforms>* GetBoneTransformsBuffer() { return BoneTransformsBuffer; }
	bool GetIsSkinned() { return IsSkined; }

	size_t GetIndexCount() { return IndexCount; }


	bool GetIsInstance(){return IsInstance;}
	FRHIColorResource* GetInstantceTexture(){return InstatnceDataResource.lock().get();}
	void SetInstantceTexture(std::shared_ptr<FRHIColorResource> InTexture){ InstatnceDataResource = InTexture;}

protected:
	IRHIConstantBuffer<FObjectConstants>* ConstantBuffer;

	//used for skinned mesh
	bool IsSkined;
	IRHIConstantBuffer<FBoneTransforms>* BoneTransformsBuffer;

	std::weak_ptr<IRHIVertexBuffer> VertexBuffer;
	std::weak_ptr<IRHIIndexBuffer> IndexBuffer;

	//used by instanced static mesh
	bool IsInstance;
	std::weak_ptr<FRHIColorResource> InstatnceDataResource;

	int VertexStrideSize = 0;
	size_t VertexBufferSize = 0;
	size_t IndexBufferSize = 0;
	size_t IndexCount = 0;
	int InstanceCount = 0;
	int VertexBaseLocation = 0;
	int StartInstanceLocation = 0;
	//reserved for material
};