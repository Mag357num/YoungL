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
		bNeedUploadInstanceData = false;

		InstanceCount = 1;
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
	//virtual void BuildInstanceBuffer(std::vector<FInstanceData>& InstanceDatas, IRHIContext* Context){}
	virtual void BuildSkinnedBoneTransBuffer(FBoneTransforms* InTransforms, IRHIContext* Context) {}
	

	virtual std::shared_ptr<FRHIColorResource> BuildInstanceBuffer(std::vector<FColor>& Colors, UINT Width, UINT Height, IRHIContext* Context){return nullptr;}
	virtual void UploadInstanceDataToBuffer(IRHIContext* Context){}
	virtual void MarkInstanceDataDirty(std::vector<FColor>& Colors, UINT Width, UINT Height){}

	virtual std::shared_ptr<IRHIVertexBuffer> BuildVertexBuffer(std::vector<FVertex>& InVertices) { return nullptr; }
	virtual std::shared_ptr<IRHIVertexBuffer> BuildVertexBuffer(std::vector<FSkinVertex>& InVertices) {return nullptr;}
	virtual std::shared_ptr<IRHIIndexBuffer> BuildIndexBuffer(std::vector<uint32_t>& InIndices) { return nullptr; }

	IRHIVertexBuffer* GetVertexBuffer() { return VertexBuffer.lock().get(); }
	IRHIIndexBuffer* GetIndexBuffer() { return IndexBuffer.lock().get(); }
	void SetVertexBuffer(std::shared_ptr<IRHIVertexBuffer> InVertexBuffer){ VertexBuffer = InVertexBuffer;}
	void SetIndexBuffer(std::shared_ptr<IRHIIndexBuffer> InIndexBuffer){IndexBuffer = InIndexBuffer;}


	IRHIConstantBuffer<FObjectConstants>* GetConstantBuffer() { return ConstantBuffer; }

	IRHIConstantBuffer<FBoneTransforms>* GetBoneTransformsBuffer() { return BoneTransformsBuffer; }
	bool GetIsSkinned() { return IsSkined; }

	size_t GetVertexStrideSize(){return VertexStrideSize;}
	size_t GetVertexBufferSize(){return VertexBufferSize;}
	size_t GetIndexBufferSize(){return IndexBufferSize;}
	size_t GetIndexCount() { return IndexCount; }
	size_t GetInstanceCount(){return InstanceCount;}

	void SetVertexStrideSize(size_t InStride) { VertexStrideSize = InStride; }
	void SetVertexBufferSize(size_t InVBSize) { VertexBufferSize = InVBSize; }
	void SetIndexBufferSize(size_t InIBSize) { IndexBufferSize = InIBSize; }
	void SetIndexCount(size_t InCount) { IndexCount = InCount; }
	void SetInstanceCount(size_t InCount) {
		InstanceCount = InCount;
		}

	bool GetIsInstance(){return IsInstance;}
	bool GetNeedUploadInstanceData(){return bNeedUploadInstanceData;}
	void SetNeedUploadInstanceData(bool InFlag){bNeedUploadInstanceData = InFlag;}
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
	bool bNeedUploadInstanceData;
	std::weak_ptr<FRHIColorResource> InstatnceDataResource;

	size_t VertexStrideSize = 0;
	size_t VertexBufferSize = 0;
	size_t IndexBufferSize = 0;
	size_t IndexCount = 0;
	size_t InstanceCount = 0;
	int VertexBaseLocation = 0;
	int StartInstanceLocation = 0;
	//reserved for material
};