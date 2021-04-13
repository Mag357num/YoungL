#include "GameCore.h"
#include <fstream>

FGameCore::FGameCore()
{
	Camera = std::make_unique<FCamera>();
}

FGameCore::~FGameCore()
{
	Camera.reset();
}

void FGameCore::OnKeyDown(UINT8 Key)
{

}

void FGameCore::OnKeyUp(UINT8 Key)
{

}

void FGameCore::OnMouseButtonDown(WPARAM BtnState, int X, int Y)
{

}

void FGameCore::OnMouseButtonUp(WPARAM BtnState, int X, int Y)
{

}

void FGameCore::OnMouseMove(WPARAM BtnState, int X, int Y)
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


static void UpdateSceneConstantBuffer_RenderThread(FMatrix* InView, FMatrix* InProj, FVector4D* InCamerLoc)
{
	FRenderThreadManager::UpdateSceneConstantBuffer(*InView, *InProj, *InCamerLoc);
}

void FGameCore::Tick()
{
	//Tick Game Logic...
	//todo:

	//update camera canstant
	if (Camera->CameraInfoDirty())
	{
		//todo:enqueue render command to update scene constant buffer
		//if (!RenderThreadManager_Weak.expired())
		//{
		//	FMatrix* View = Camera->GetCameraView();
		//	FMatrix* Proj = Camera->GetCameraProj();
		//	FVector4D* CamLoc = Camera->GetCameraLoc();

		//	FRenderThreadCommand CreateRenderItemCommand;
		//	CreateRenderItemCommand.Wrap(UpdateSceneConstantBuffer_RenderThread,
		//		View, Proj, CamLoc);

		//	std::shared_ptr<FRenderThreadManager> RenderManager = RenderThreadManager_Weak.lock();
		//	RenderManager->PushRenderCommand(CreateRenderItemCommand);
		//}

		Camera->ResetDirtyFlat();
	}
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