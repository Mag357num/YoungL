#include "pch.h"
#include "GameCore.h"

#include "ModelLoader.h"

UGameCore::UGameCore(int ViewWidth, int ViewHeigt)
{
	Camera = std::make_shared<UCamera>(ViewWidth, ViewHeigt);

}

UGameCore::~UGameCore()
{
	Camera.reset();

	if (SceneConstant)
	{
		delete SceneConstant;
		SceneConstant = nullptr;
	}
}

void UGameCore::Initialize()
{
	AssetManager = new UGameAssetManager();

	PlayerInput = new UPlayerInput();
	PlayerInput->SetBindedCamera(Camera);


	AssetPaths.push_back(L"Models/ModelFloor.Bin");
	AssetPaths.push_back(L"Models/ModelSave.Bin");


	//TODO: Scene Management
	for (size_t Index = 0; Index < AssetPaths.size(); ++Index)
	{
		LoadActor(AssetPaths[Index]);

		//test modify 
		if (Index == 0)
		{
			StaticActors[0]->InitiallySetLocation(FVector(0.0f, 0.0f, -20.0f));
		}
	}

	//SkinedPaths.push_back(L"Models/soldier.m3d");
	SkinedPaths.push_back(L"Models/Skeleton.m3d");
	for (size_t Index = 0; Index < SkinedPaths.size(); ++Index)
	{
		LoadActor(SkinedPaths[Index], true);

		//test modify 
		if (Index == 0)
		{
			SkinedActors[0]->InitiallySetLocation(FVector(200.0f, 0.0f, 0.0f));
			SkinedActors[0]->InitiallySetScaling(FVector(1.0f, 1.0f, 1.0f));
			SkinedActors[0]->InitiallySetRotation(FVector4D(0.0f, 0.0f, 3.14f, 0.0f));

			PlayerInput->SetBindedCharacter(SkinedActors[0]);
		}
	}
	

	//test for InstancedStaticMeshActor
	//"Models/skull.txt"
	{
		std::wstring InstancedMeshPath = L"Models/skull.txt";
		std::unique_ptr<FGeometry<FVertex>> InstancedGeo = FModelLoader::BuildSkullGeometry(InstancedMeshPath);

		std::shared_ptr<UStaticMesh> InstanceStatic = std::make_shared<UStaticMesh>("InstancedStaticMesh");
		InstanceStatic->SetAssetPath(InstancedMeshPath);
		InstanceStatic->SetGeometry(std::move(InstancedGeo));
		AssetManager->AddStaticMesh(InstancedMeshPath, InstanceStatic);

		if (InstanceStatic)
		{
			int RandomInt = rand();
			std::string Name = "InstancedStaticMeshActor";
			Name += to_string(RandomInt);

			std::shared_ptr<AInstancedStaticMeshActor> TestInstanceActor = std::make_shared<AInstancedStaticMeshActor>(Name);
			TestInstanceActor->SetStaticMesh(InstanceStatic);

			TestInstanceActor->InitiallySetLocation(FVector(200.0f, 200.0f, 0.0f));

			//test instance
			for (int Col = 0; Col < 32; ++Col)
			{
				for (int Row = 0; Row < 32; ++Row)
				{
					FActorInstanceInfo InstanceInfo;
					InstanceInfo.Rotation = FVector4D(0.0f, 0.0f, 0.0f, 0.0f);
					InstanceInfo.Translation = FVector4D(Col * 200.0f, Row * 200.0f, 0.0f, 0.0f);
					float Rand = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
					float Scale = 15.0f + 2.0f * Rand;
					InstanceInfo.Scaling = FVector4D(Scale, Scale, Scale, 1.0f);
					TestInstanceActor->AddInstance(InstanceInfo);
				}
			}

			InstanceStaticActors.push_back(TestInstanceActor);
		}
	}


	//std::shared_ptr<UStaticMesh> InstanceStatic = AssetManager->CheckStaticMeshLoaded(AssetPaths[1]);
	//if (InstanceStatic)
	//{
	//	int RandomInt = rand();
	//	std::string Name = "InstancedStaticMeshActor";
	//	Name += to_string(RandomInt);

	//	std::unique_ptr<AInstancedStaticMeshActor> TestInstanceActor = std::make_unique<AInstancedStaticMeshActor>(Name);
	//	TestInstanceActor->SetStaticMesh(InstanceStatic);

	//	TestInstanceActor->InitiallySetLocation(FVector(200.0f, 200.0f, 0.0f));

	//	//test instance
	//	for (int Col = 0; Col < 1; ++Col)
	//	{
	//		for (int Row = 0; Row < 1; ++Row)
	//		{
	//			FActorInstanceInfo InstanceInfo;
	//			InstanceInfo.Rotation = FVector4D(0.0f, 0.0f, 0.0f, 0.0f);
	//			InstanceInfo.Translation = FVector4D(Col * 300.0f, Row * 300.0f, 0.0f, 0.0f);
	//			float Rand = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
	//			float Scale = 0.8f + 0.2f * Rand;
	//			InstanceInfo.Scaling = FVector4D(Scale, Scale, Scale, 1.0f);
	//			TestInstanceActor->AddInstance(InstanceInfo);
	//		}
	//	}

	//	InstanceStaticActors.push_back(std::move(TestInstanceActor));
	//}
	
}


static void UpdateSceneConstantBuffer_RenderThread(FSceneConstant* SceneConstant)
{
	FRenderThreadManager::UpdateSceneConstantBuffer(SceneConstant);
}

void UGameCore::Tick(float DeltaTime)
{
	
	//Tick player input
	if (PlayerInput)
	{
		PlayerInput->Tick(DeltaTime);
	}

	//Utilities::Print(L"Game Thread Tick.....\n");
	
	//Tick Game Logic...
	//todo:
	//deal with Rotate event


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

			FRenderThreadCommand UpdateSceneConstantCommand;
			UpdateSceneConstantCommand.Wrap(UpdateSceneConstantBuffer_RenderThread, SceneConstant);

			std::shared_ptr<FRenderThreadManager> RenderManager = RenderThreadManager_Weak.lock();
			RenderManager->PushRenderCommand(UpdateSceneConstantCommand);
		}

		Camera->ResetDirtyFlat();
	}

	//Tick Actors 
	//TODO: TickGroup
	for (int StaticIndex = 0; StaticIndex < StaticActors.size(); ++StaticIndex)
	{
		StaticActors[StaticIndex]->Tick(DeltaTime);
	}
	for (int SkinIndex = 0; SkinIndex < SkinedActors.size(); ++SkinIndex)
	{
		SkinedActors[SkinIndex]->Tick(DeltaTime);
	}
	for (int InstanceActorIndex = 0; InstanceActorIndex < InstanceStaticActors.size(); InstanceActorIndex++)
	{
		InstanceStaticActors[InstanceActorIndex]->Tick(DeltaTime);
	}
}

void UGameCore::LoadActor(std::wstring& Path, bool bSkinedActor)
{
	int RandomInt = rand();

	if (bSkinedActor)
	{
		std::string Name = "SkinnedMeshActor";
		Name += to_string(RandomInt);
		std::shared_ptr<ASkeletalMeshActor> SkinedActor = std::make_shared<ASkeletalMeshActor>(Name);

		std::shared_ptr<USkeletalMesh> SkeletalMesh = AssetManager->CheckSkeletalMeshLoaded(Path);
		if (!SkeletalMesh)
		{
			std::unique_ptr<FGeometry<FSkinVertex>> SkinGeo = FModelLoader::LoadSkinedMeshAndAnimation(Path, SkinedActor->GetSkinedData());

			SkeletalMesh = std::make_shared<USkeletalMesh>("SkeletalMesh");
			SkeletalMesh->SetAssetPath(Path);
			SkeletalMesh->SetGeometry(std::move(SkinGeo));
			AssetManager->AddSkeletalMesh(Path, SkeletalMesh);
		}

		SkinedActor->SetSkeletalMesh(SkeletalMesh);
		//SkinedActor->SetSkinGeometry(SkinGeo);
		SkinedActor->TestPlayAnimation();//test play
		SkinedActors.push_back(SkinedActor);

	}
	else
	{
		
		std::string Name = "StaticMeshActor";
		Name += to_string(RandomInt);
		std::shared_ptr<AStaticMeshActor> GeoActor = std::make_shared<AStaticMeshActor>(Name);

		std::shared_ptr<UStaticMesh> StaticMesh = AssetManager->CheckStaticMeshLoaded(Path);
		if (!StaticMesh)
		{
			std::unique_ptr<FGeometry<FVertex>> Geo = FModelLoader::LoadStaticMesh(Path);

			StaticMesh = std::make_shared<UStaticMesh>("StaticMesh");
			StaticMesh->SetAssetPath(Path);
			StaticMesh->SetGeometry(std::move(Geo));
			AssetManager->AddStaticMesh(Path, StaticMesh);
		}
		

		GeoActor->SetStaticMesh(StaticMesh);
		StaticActors.push_back(GeoActor);
	}
	

}