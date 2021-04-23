#pragma once
#include "../RHIContext.h"
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
	virtual void BuildIndexBuffer(std::vector<uint16_t>& InIndices)override;
	virtual void BuildVertexBuffer(std::vector<FVertex>& InVertices)override;
	virtual void BuildVertexBuffer(std::vector<FSkinVertex>& InVertices)override;
private:

};
