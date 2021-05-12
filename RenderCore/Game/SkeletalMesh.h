#pragma once

class USkeletalMesh
{
public:
	USkeletalMesh() {}
	~USkeletalMesh() {}

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
