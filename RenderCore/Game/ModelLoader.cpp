#include "pch.h"
#include "ModelLoader.h"
#include <fstream>

std::unique_ptr<FGeometry<FVertex>> FModelLoader::LoadStaticMesh(std::string& Path)
{
	//load asset
	std::vector<FVertex> Vertices;
	std::ifstream Fin(Path, std::ios::in | std::ios::binary);

	int VertexNum;
	Fin.read((char*)&VertexNum, sizeof(int));
	Vertices.resize(VertexNum);
	Fin.read((char*)Vertices.data(), sizeof(FVertex) * VertexNum);

	std::vector<uint32_t> Indices;
	int IndexNum;
	Fin.read((char*)&IndexNum, sizeof(int));
	Indices.resize(IndexNum);
	Fin.read((char*)Indices.data(), sizeof(int) * IndexNum);

	Fin.close();

	std::unique_ptr<FGeometry<FVertex>> Geo = std::make_unique<FGeometry<FVertex>>(Vertices, Indices);

	return std::move(Geo);
}

std::unique_ptr<FGeometry<FSkinVertex>> FModelLoader::LoadSkinedMeshAndAnimation(std::string& Path, FSkinedData* OutSkinData)
{
	return nullptr;
}