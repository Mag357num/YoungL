#pragma once
#include "../Utilities.h"
#include "SkinData.h"


class FModelLoader
{
public:
	FModelLoader(){}
	~FModelLoader(){}

	struct FSubset
	{
		UINT Id = -1;
		UINT VertexStart = 0;
		UINT VertexCount = 0;
		UINT FaceStart = 0;
		UINT FaceCount = 0;
	};

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

	static void ReadMaterials(std::ifstream& Fin, UINT NumMaterials, std::vector<FMaterial>& Mats);
	static void ReadSubsetTable(std::ifstream& Fin, UINT NumSubSets, std::vector<FSubset>& Subsets);
	static void ReadSkinnedVertices(std::ifstream& Fin, UINT NumVertices, std::vector<FSkinVertex>& OutVertices);
	static void ReadTriangles(std::ifstream& Fin, UINT NumTriangles, std::vector<USHORT>& OutIndices);
	static void ReadBoneOffsets(std::ifstream& Fin, UINT NumBones, std::vector<FMatrix>& BoneOffsets);
	static void ReadBoneHierarchy(std::ifstream& Fin, UINT NumBones, std::vector<int>& BoneIndexToParentIndex);
	static void ReadAnimationClips(std::ifstream& Fin, UINT NumBones, UINT NumAnimationClips, 
										std::unordered_map<std::string, FAnimationClip>& AnimationClips);
	static void ReadBoneKeyFrames(std::ifstream& Fin, UINT NumBones, FBoneAnimation& Animation);

};