#pragma once

#include <intsafe.h>
#include <wtypes.h>
#include "StaticMeshActor.h"
#include "SkeletalMeshActor.h"
#include "InstancedStaticMeshActor.h"

#include <string>
#define  AssetPathFLOOR L"Models/ModelFloor.Bin"
#define  AssetPathModel L"Models/ModelSave.Bin"

#include "Camera.h"
#include "GameAssetManager.h"
#include "../Render/RenderThreadManager.h"

class FGameCore
{
public:
	FGameCore(int ViewWidth, int ViewHeigt);
	~FGameCore();

	FGameCore(){}

	virtual void Initialize();
	virtual void ShutDown()
	{ 
		if (StaticActors.size() > 0)
		{
			for (size_t Index = 0; Index < StaticActors.size(); Index++)
			{
				StaticActors[Index].reset();
			}
			if (!StaticActors.empty())
			{
				printf("Empty Error!");
			}
		}
		

		if (SkinedActors.size() > 0)
		{
			for (size_t Index = 0; Index < SkinedActors.size(); Index++)
			{
				SkinedActors[Index].reset();
			}
			if (!SkinedActors.empty())
			{
				printf("Empty Error!");
			}

		}
		

		if (InstanceStaticActors.size() > 0)
		{
			for (size_t Index = 0; Index < InstanceStaticActors.size(); Index++)
			{
				InstanceStaticActors[Index].reset();
			}
			if (!InstanceStaticActors.empty())
			{
				printf("Empty Error!");
			}
		}

		if (!AssetPaths.empty())
		{
			printf("Empty Error!");
		}

		if (!SkinedPaths.empty())
		{
			printf("Empty Error!");
		}

		if (SceneConstant)
		{
			delete SceneConstant;
			SceneConstant = nullptr;
		}

		if (AssetManager)
		{
			delete AssetManager;
			AssetManager = nullptr;
		}
	}

	virtual void Tick(float DeltaTime);

	virtual void OnKeyDown(UINT8 Key);
	virtual void OnKeyUp(UINT8 Key);

	virtual void OnMouseButtonDown(WPARAM BtnState, int X, int Y);
	virtual void OnMouseButtonUp(WPARAM BtnState, int X, int Y);
	virtual void OnMouseMove(WPARAM BtnState, int X, int Y);

	std::vector<std::unique_ptr<AStaticMeshActor>>& GetStaticActors() {
		return StaticActors;}

	std::vector<std::unique_ptr<ASkeletalMeshActor>>& GetSkinedActors() {
		return SkinedActors;
	}
	std::vector<std::unique_ptr<AInstancedStaticMeshActor>>& GetInstancedStaticMeshActors() {
		return InstanceStaticActors;
	}

	std::weak_ptr<FRenderThreadManager> RenderThreadManager_Weak;
private:
	UGameAssetManager* AssetManager;
	
	void LoadActor(std::wstring& Path, bool bSkinedActor = false);

	//unique_ptr will be automated released when ~FGameCore
	std::vector<std::unique_ptr<AStaticMeshActor>> StaticActors;
	std::vector<std::unique_ptr<ASkeletalMeshActor>> SkinedActors;
	std::vector<std::unique_ptr<AInstancedStaticMeshActor>> InstanceStaticActors;

	std::vector<std::wstring> AssetPaths;
	std::vector<std::wstring> SkinedPaths;

	std::unique_ptr<FCamera> Camera;

	FSceneConstant* SceneConstant;

	//mouse position
	FVector2D MousePosition;
	//saved to update delta x && delta Y in window
	POINT WindowOffset;

	bool bMouseButtonDown;

};

