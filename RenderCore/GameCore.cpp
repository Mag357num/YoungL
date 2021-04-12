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
	AssetPaths.push_back("Models/ModelFloor.Bin");
	AssetPaths.push_back("Models/ModelSave.Bin");

	for (size_t Index = 0; Index < AssetPaths.size(); ++Index)
	{
		LoadAsset(AssetPaths[Index]);
	}
	
}

void FGameCore::Tick()
{
	//Tick Game Logic...
	//todo:
}

void FGameCore::LoadAsset(std::string& Path)
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

	//create resource
	std::unique_ptr<FGeometry> Geo = std::make_unique<FGeometry>(Vertices, Indices);
	std::unique_ptr<AMeshActor> GeoActor = std::make_unique<AMeshActor>();
	GeoActor->SetGeometry(Geo);
	Geometries.push_back(std::move(GeoActor));
}