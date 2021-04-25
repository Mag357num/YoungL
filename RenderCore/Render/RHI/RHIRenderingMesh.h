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
	}

	virtual ~IRHIRenderingMesh() {

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

	virtual void BuildConstantBuffer(FObjectConstants* InObjConstants, IRHIContext* Context) {}

	virtual void BuildSkinnedBoneTransBuffer(FBoneTransforms* InTransforms, IRHIContext* Context) {}

	virtual void BuildVertexBuffer(std::vector<FVertex>& InVertices) {}

	virtual void BuildVertexBuffer(std::vector<FSkinVertex>& InVertices) {}

	virtual void BuildIndexBuffer(std::vector<uint16_t>& InIndices) {}

	IRHIVertexBuffer* GetVertexBuffer() { return VertexBuffer; }
	IRHIIndexBuffer* GetIndexBuffer() { return IndexBuffer; }
	IRHIConstantBuffer<FObjectConstants>* GetConstantBuffer() { return ConstantBuffer; }

	IRHIConstantBuffer<FBoneTransforms>* GetBoneTransformsBuffer() { return BoneTransformsBuffer; }
	bool GetIsSkinned() { return IsSkined; }

	size_t GetIndexCount() { return IndexCount; }

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