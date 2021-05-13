#pragma once

#include "../Render/RHI/RHIVertexBuffer.h"
#include "../Render/RHI/RHIIndexBuffer.h"
#include "Object.h"

class UStaticMesh : public UObject
{
public:
	UStaticMesh(std::string TypeName)
		:UObject(TypeName)
	{

	}
	virtual ~UStaticMesh(){
	}

	virtual void Serialize()override {}

	virtual void PostLoad()override {}

	virtual void Destroy()override {}

	void SetAssetPath(std::wstring Inpath){ AssetPath = Inpath;}
	void SetGeometry(std::unique_ptr<FGeometry<FVertex>> InGeometry)
	{
		Geometry = std::move(InGeometry);
	}

	FGeometry<FVertex>* GetGeometry() {
		return Geometry.get();
	}

	std::wstring GetAssetPath(){return AssetPath;}

private:
	std::wstring AssetPath;
	std::unique_ptr<FGeometry<FVertex>> Geometry;


};
