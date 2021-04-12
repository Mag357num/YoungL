#pragma once

#include <intsafe.h>
#include "MeshActor.h"
#include <string>
#define  AssetPathFLOOR L"Models/ModelFloor.Bin"
#define  AssetPathModel L"Models/ModelSave.Bin"

class FGameCore
{
public:
	FGameCore(){}
	~FGameCore(){}

	virtual void Initialize();
	virtual void ShutDown(){ Geometries.empty();
	AssetPaths.empty();
	}

	virtual void Tick();

	virtual void OnKeyDown(UINT8 Key);
	virtual void OnKeyUp(UINT8 Key);

	std::vector<std::unique_ptr<AMeshActor>>& GetGeometries() {
		return Geometries;}
private:
	void LoadAsset(std::string& Path);

	std::vector<std::unique_ptr<AMeshActor>> Geometries;

	std::vector<std::string> AssetPaths;

};

