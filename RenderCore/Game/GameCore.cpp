#include "pch.h"
#include "GameCore.h"

#include "ModelLoader.h"

FGameCore::FGameCore(int ViewWidth, int ViewHeigt)
{
	Camera = std::make_unique<FCamera>(ViewWidth, ViewHeigt);

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

	POINT CurPoint = { 0, 0 };
	GetCursorPos(&CurPoint);
	WindowOffset.x = CurPoint.x - X;
	WindowOffset.y = CurPoint.y - Y;

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
		//get window offset
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
	AssetManager = new UGameAssetManager();

	AssetPaths.push_back(L"Models/ModelFloor.Bin");
	AssetPaths.push_back(L"Models/ModelSave.Bin");

	for (size_t Index = 0; Index < AssetPaths.size(); ++Index)
	{
		LoadActor(AssetPaths[Index]);

		//test modify 
		if (Index == 0)
		{
			StaticActors[0]->InitiallySetLocation(FVector(0.0f, 0.0f, -20.0f));
		}
	}

	SkinedPaths.push_back(L"Models/soldier.m3d");
	for (size_t Index = 0; Index < SkinedPaths.size(); ++Index)
	{
		LoadActor(SkinedPaths[Index], true);

		//test modify 
		if (Index == 0)
		{
			SkinedActors[0]->InitiallySetLocation(FVector(200.0f, 0.0f, 0.0f));
			SkinedActors[0]->InitiallySetScaling(FVector(3.0f, 3.0f, 3.0f));
			SkinedActors[0]->InitiallySetRotation(FVector4D(1.57f, 0.0f, 0.0f, 0.0f));
		}
	}
}


static void UpdateSceneConstantBuffer_RenderThread(FSceneConstant* SceneConstant)
{
	FRenderThreadManager::UpdateSceneConstantBuffer(SceneConstant);
}

void FGameCore::Tick(float DeltaTime)
{

	//Utilities::Print(L"Game Thread Tick.....\n");
	
	//Tick Game Logic...
	//todo:
	//deal with Rotate event
	if (bMouseButtonDown)
	{
		POINT CurPoint = { 0, 0 };
		GetCursorPos(&CurPoint);
		OnMouseMove(WM_LBUTTONDOWN, CurPoint.x - WindowOffset.x, CurPoint.y - WindowOffset.y);
	}

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
			SceneConstant->ViewProj = FMath::MatrixTranspose(SceneConstant->ViewProj);
			SceneConstant->CamLocation = FVector4D(CamLoc.X, CamLoc.Y, CamLoc.Z, 1.0f);

			FRenderThreadCommand CreateRenderItemCommand;
			CreateRenderItemCommand.Wrap(UpdateSceneConstantBuffer_RenderThread, SceneConstant);

			std::shared_ptr<FRenderThreadManager> RenderManager = RenderThreadManager_Weak.lock();
			RenderManager->PushRenderCommand(CreateRenderItemCommand);
		}

		Camera->ResetDirtyFlat();
	}

	//Tick Actors 
	//todo: TickGroup
	for (int StaticIndex = 0; StaticIndex < StaticActors.size(); ++StaticIndex)
	{
		StaticActors[StaticIndex]->Tick(DeltaTime);
	}

	for (int SkinIndex = 0; SkinIndex < SkinedActors.size(); ++SkinIndex)
	{
		SkinedActors[SkinIndex]->Tick(DeltaTime);
	}
}

void FGameCore::LoadActor(std::wstring& Path, bool bSkinedActor)
{
	int RandomInt = rand();

	if (bSkinedActor)
	{
		std::string Name = "SkinnedMeshActor";
		Name += to_string(RandomInt);
		std::unique_ptr<ASkeletalMeshActor> SkinedActor = std::make_unique<ASkeletalMeshActor>(Name);

		std::shared_ptr<USkeletalMesh> SkeletalMesh = AssetManager->CheckSkeletalMeshLoaded(Path);
		if (!SkeletalMesh)
		{
			std::unique_ptr<FGeometry<FSkinVertex>> SkinGeo = FModelLoader::LoadSkinedMeshAndAnimation(Path, SkinedActor->GetSkinedData());

			SkeletalMesh = std::make_shared<USkeletalMesh>();
			SkeletalMesh->SetAssetPath(Path);
			SkeletalMesh->SetGeometry(std::move(SkinGeo));
			AssetManager->AddSkeletalMesh(Path, SkeletalMesh);
		}

		SkinedActor->SetSkeletalMesh(SkeletalMesh);
		//SkinedActor->SetSkinGeometry(SkinGeo);
		SkinedActor->TestPlayAnimation();//test play
		SkinedActors.push_back(std::move(SkinedActor));

	}
	else
	{
		
		std::string Name = "StaticMeshActor";
		Name += to_string(RandomInt);
		std::unique_ptr<AStaticMeshActor> GeoActor = std::make_unique<AStaticMeshActor>(Name);

		std::shared_ptr<UStaticMesh> StaticMesh = AssetManager->CheckStaticMeshLoaded(Path);
		if (!StaticMesh)
		{
			std::unique_ptr<FGeometry<FVertex>> Geo = FModelLoader::LoadStaticMesh(Path);

			StaticMesh = std::make_shared<UStaticMesh>();
			StaticMesh->SetAssetPath(Path);
			StaticMesh->SetGeometry(std::move(Geo));
			AssetManager->AddStaticMesh(Path, StaticMesh);
		}
		

		GeoActor->SetStaticMesh(StaticMesh);
		StaticActors.push_back(std::move(GeoActor));
	}
	

}