#include "GameCore.h"
#include <fstream>

void FGameCore::OnKeyDown(UINT8 Key)
{

}

void FGameCore::OnKeyUp(UINT8 Key)
{

}

void FGameCore::Initialize()
{
	LoadAssets();
}

void FGameCore::Tick()
{

}

void FGameCore::LoadAssets()
{
	//load asset
	std::vector<FVertex> Vertices;
	std::ifstream Fin(AssetPath, std::ios::in | std::ios::binary);

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

	//create resource
	std::unique_ptr<FGeometry> Geo = std::make_unique<FGeometry>(Vertices, Indices);
	std::unique_ptr<AMeshActor> GeoActor = std::make_unique<AMeshActor>();
	Geometries.push_back(std::move(GeoActor));
}