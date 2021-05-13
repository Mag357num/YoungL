#pragma once

#include "../Render/RHI/RHIVertexBuffer.h"
#include "../Render/RHI/RHIIndexBuffer.h"
#include "Object.h"

class USkeletalMesh : public UObject
{
public:
	USkeletalMesh(std::string TypeName)
		:UObject(TypeName)
	{

	}

	virtual ~USkeletalMesh() {

	}

	virtual void Serialize()override {}

	virtual void PostLoad()override {}

	virtual void Destroy()override {}

	void SetAssetPath(std::wstring Inpath) { AssetPath = Inpath; }
	void SetGeometry(std::unique_ptr<FGeometry<FSkinVertex>> InGeometry)
	{
		Geometry = std::move(InGeometry);
	}

	FGeometry<FSkinVertex>* GetGeometry() {
		return Geometry.get();
	}

	std::wstring GetAssetPath() { return AssetPath; }

private:
	std::wstring AssetPath;
	std::unique_ptr<FGeometry<FSkinVertex>> Geometry;


};
