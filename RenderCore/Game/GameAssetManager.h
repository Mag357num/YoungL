#pragma once

#include <map>
#include "StaticMesh.h"
#include "SkeletalMesh.h"

class UGameAssetManager
{
public:
	UGameAssetManager(){}
	~UGameAssetManager(){

		if (StaticMeshes.size() > 0)
		{
			for (auto It = StaticMeshes.begin(); It != StaticMeshes.end(); ++It)
			{
				It->second.reset();
			}
			StaticMeshes.clear();
			if (!StaticMeshes.empty())
			{
				Utilities::Print("GameAssetManager Empty StaticMeshes Error! \n");
			}

			for (auto ItSkeletal = SkeletalMeshes.begin(); ItSkeletal != SkeletalMeshes.end(); ++ItSkeletal)
			{
				ItSkeletal->second.reset();
			}

			SkeletalMeshes.clear();
			if (!SkeletalMeshes.empty())
			{
				Utilities::Print("GameAssetManager Empty SkeletalMeshes Error! \n");
			}
		}
	}


	void AddStaticMesh(std::wstring Path, std::shared_ptr<UStaticMesh> InMesh)
	{ 
		StaticMeshes.insert(std::make_pair(Path, InMesh));
	}

	std::shared_ptr<UStaticMesh> CheckStaticMeshLoaded(std::wstring Path)
	{

		std::map<std::wstring, std::shared_ptr<UStaticMesh>>::iterator Iter;
		Iter = StaticMeshes.find(Path);
		if (Iter != StaticMeshes.end())
		{
			return Iter->second;
		}


		return nullptr;
	}

	void AddSkeletalMesh(std::wstring Path, std::shared_ptr<USkeletalMesh> InMesh)
	{
		SkeletalMeshes.insert(std::make_pair(Path, InMesh));
	}

	std::shared_ptr<USkeletalMesh> CheckSkeletalMeshLoaded(std::wstring Path)
	{

		std::map<std::wstring, std::shared_ptr<USkeletalMesh>>::iterator Iter;
		Iter = SkeletalMeshes.find(Path);
		if (Iter != SkeletalMeshes.end())
		{
			return Iter->second;
		}


		return nullptr;
	}

private:
	std::map<std::wstring, std::shared_ptr<UStaticMesh>> StaticMeshes;
	std::map<std::wstring, std::shared_ptr<USkeletalMesh>> SkeletalMeshes;
};
