#include "GameCore.h"
#include <fstream>

FGameCore::FGameCore()
{
	Camera = std::make_unique<FCamera>();

	bMouseButtonDown = false;
	MousePosition = FVector2D(0.0f, 0.0f);
}

FGameCore::~FGameCore()
{
	Camera.reset();

	if (SceneConstant)
	{
		delete SceneConstant;
		SceneConstant = nullptr;
	}
}

void FGameCore::OnKeyDown(UINT8 Key)
{

}

void FGameCore::OnKeyUp(UINT8 Key)
{

}

void FGameCore::OnMouseButtonDown(WPARAM BtnState, int X, int Y)
{
	MousePosition.X = (float)X;
	MousePosition.Y = (float)Y;
	bMouseButtonDown = true;
}

void FGameCore::OnMouseButtonUp(WPARAM BtnState, int X, int Y)
{
	bMouseButtonDown = false;
}

void FGameCore::OnMouseMove(WPARAM BtnState, int X, int Y)
{
	if (bMouseButtonDown)
	{
		float Dx = X - MousePosition.X;
		float Dy = Y - MousePosition.Y;
		
		Camera->Pitch(Dy * 0.2f);
		Camera->Rotate(Dx * 0.2f);


		MousePosition.X = (float)X;
		MousePosition.Y = (float)Y;
	}
}

void FGameCore::Initialize()
{
	AssetPaths.push_back("Models/ModelFloor.Bin");
	AssetPaths.push_back("Models/ModelSave.Bin");

	for (size_t Index = 0; Index < AssetPaths.size(); ++Index)
	{
		LoadAsset(AssetPaths[Index]);

		//test modify 
		if (Index == 0)
		{
			Geometries[0]->SetLocation(FVector(0.0f, 0.0f, -50.0f));
		}
	}
	
}


static void UpdateSceneConstantBuffer_RenderThread(FSceneConstant* SceneConstant)
{
	FRenderThreadManager::UpdateSceneConstantBuffer(SceneConstant);
}

void FGameCore::Tick()
{
	//Tick Game Logic...
	//todo:

	//update camera canstant
	if (Camera->CameraInfoDirty())
	{
		//todo:enqueue render command to update scene constant buffer
		if (!RenderThreadManager_Weak.expired())
		{
			if (!SceneConstant)
			{
				SceneConstant = new FSceneConstant();
			}

			FMatrix View = *Camera->GetCameraView();
			FMatrix Proj = *Camera->GetCameraProj();
			FVector4D CamLoc = *Camera->GetCameraLoc();

			SceneConstant->ViewProj = View * Proj;

			//copy to upload buffer transposed???
			SceneConstant->ViewProj = Utilities::MatrixTranspose(SceneConstant->ViewProj);
			SceneConstant->CamLocation = FVector4D(CamLoc.X, CamLoc.Y, CamLoc.Z, 1.0f);

			FRenderThreadCommand CreateRenderItemCommand;
			CreateRenderItemCommand.Wrap(UpdateSceneConstantBuffer_RenderThread, SceneConstant);

			std::shared_ptr<FRenderThreadManager> RenderManager = RenderThreadManager_Weak.lock();
			RenderManager->PushRenderCommand(CreateRenderItemCommand);
		}

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