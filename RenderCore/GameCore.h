#pragma once

#include <intsafe.h>
#include <wtypes.h>
#include "MeshActor.h"
#include <string>
#define  AssetPathFLOOR L"Models/ModelFloor.Bin"
#define  AssetPathModel L"Models/ModelSave.Bin"

#include "Camera.h"
#include "RenderThreadManager.h"

class FGameCore
{
public:
	FGameCore();
	~FGameCore();

	virtual void Initialize();
	virtual void ShutDown()
	{ 
		if (!Geometries.empty())
		{
			printf("Empty Error!");
		}
		if (!AssetPaths.empty())
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

	std::vector<std::unique_ptr<AMeshActor>>& GetGeometries() {
		return Geometries;}

	std::weak_ptr<FRenderThreadManager> RenderThreadManager_Weak;
private:
	void LoadAsset(std::string& Path);

	std::vector<std::unique_ptr<AMeshActor>> Geometries;

	std::vector<std::string> AssetPaths;

	std::unique_ptr<FCamera> Camera;

	FSceneConstant* SceneConstant;

	//mouse position
	FVector2D MousePosition;
	POINT WindowOffset;
	bool bMouseButtonDown;
};

