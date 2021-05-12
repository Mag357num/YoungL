#pragma once
class UStaticMesh
{
public:
	UStaticMesh(){}
	~UStaticMesh(){}

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
