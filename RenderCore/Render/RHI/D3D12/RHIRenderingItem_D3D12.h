#pragma once
#include "../RHIRenderingMesh.h"
#include "../../../Utilities.h"

class FRHIRenderingMesh_D3D12 : public IRHIRenderingMesh
{
public:
	FRHIRenderingMesh_D3D12(){}
	virtual ~FRHIRenderingMesh_D3D12()
	{

	}

	virtual void BuildConstantBuffer(FObjectConstants* InObjConstants, IRHIContext* Context)override;
	virtual void BuildSkinnedBoneTransBuffer(FBoneTransforms* InTransforms, IRHIContext* Context)override;

	virtual std::shared_ptr<FRHIColorResource> BuildInstanceBuffer(std::vector<FColor>& Colors, UINT Width, UINT Height, IRHIContext* Context)override;
	virtual void UploadInstanceDataToBuffer(IRHIContext* Context)override;
	virtual void MarkInstanceDataDirty(std::vector<FColor>& Colors, UINT Width, UINT Height)override;

	virtual std::shared_ptr<IRHIIndexBuffer> BuildIndexBuffer(std::vector<uint32_t>& InIndices)override;
	virtual std::shared_ptr<IRHIVertexBuffer> BuildVertexBuffer(std::vector<FVertex>& InVertices)override;
	virtual std::shared_ptr<IRHIVertexBuffer> BuildVertexBuffer(std::vector<FSkinVertex>& InVertices)override;


private:
	//instance texture data
	std::vector<FColor>* InstanceData;
	UINT InstanceTextureWidth;
	UINT InstanceTextureHeight;
};
