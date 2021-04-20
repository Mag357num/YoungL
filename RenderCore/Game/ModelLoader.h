#pragma once
#include "../Utilities.h"
#include "SkinData.h"

class FModelLoader
{
public:
	FModelLoader(){}
	~FModelLoader(){}

	static std::unique_ptr<FGeometry<FVertex>> LoadStaticMesh(std::string& Path);

	static void LoadSkinedMesh()
	{

	}

	static void LoadSkinedMeshAnimation()
	{

	}

	static std::unique_ptr<FGeometry<FSkinVertex>> LoadSkinedMeshAndAnimation(std::string& Path, FSkinedData* OutSkinData);

protected:
private:
};