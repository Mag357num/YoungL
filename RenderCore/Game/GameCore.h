#pragma once

#include <intsafe.h>
#include <wtypes.h>
#include "MeshActor.h"
#include "SkinMeshActor.h"
#include <string>
#define  AssetPathFLOOR L"Models/ModelFloor.Bin"
#define  AssetPathModel L"Models/ModelSave.Bin"

#include "Camera.h"
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
		}
		if (!StaticActors.empty())
		{
			printf("Empty Error!");
		}

		if (SkinedActors.size() > 0)
		{
			for (size_t Index = 0; Index < SkinedActors.size(); Index++)
			{
				SkinedActors[Index].reset();
			}
		}
		if (!SkinedActors.empty())
		{
			printf("Empty Error!");
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
	}

	virtual void Tick(float DeltaTime);

	virtual void OnKeyDown(UINT8 Key);
	virtual void OnKeyUp(UINT8 Key);

	virtual void OnMouseButtonDown(WPARAM BtnState, int X, int Y);
	virtual void OnMouseButtonUp(WPARAM BtnState, int X, int Y);
	virtual void OnMouseMove(WPARAM BtnState, int X, int Y);

	std::vector<std::unique_ptr<AMeshActor>>& GetStaticActors() {
		return StaticActors;}

	std::vector<std::unique_ptr<ASkinMeshActor>>& GetSkinedActors() {
		return SkinedActors;
	}

	std::weak_ptr<FRenderThreadManager> RenderThreadManager_Weak;
private:
	void LoadActor(std::string& Path, bool bSkinedActor = false);

	//unique_ptr will be automated released when ~FGameCore
	std::vector<std::unique_ptr<AMeshActor>> StaticActors;
	std::vector<std::unique_ptr<ASkinMeshActor>> SkinedActors;

	std::vector<std::string> AssetPaths;
	std::vector<std::string> SkinedPaths;

	std::unique_ptr<FCamera> Camera;

	FSceneConstant* SceneConstant;

	//mouse position
	FVector2D MousePosition;
	//saved to update delta x && delta Y in window
	POINT WindowOffset;

	bool bMouseButtonDown;
};

