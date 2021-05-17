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

	virtual std::shared_ptr<IRHIIndexBuffer> BuildIndexBuffer(std::vector<uint32_t>& InIndices)override;
	virtual std::shared_ptr<IRHIVertexBuffer> BuildVertexBuffer(std::vector<FVertex>& InVertices)override;
	virtual std::shared_ptr<IRHIVertexBuffer> BuildVertexBuffer(std::vector<FSkinVertex>& InVertices)override;
	virtual std::shared_ptr<IRHIVertexBuffer> BuildInstanceBuffer(std::vector<FInstanceData>& InInstanceDatas)override;

private:

};
