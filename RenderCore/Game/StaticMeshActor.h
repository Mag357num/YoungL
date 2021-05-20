#pragma once

#include "../Utilities.h"
#include "../Math/Math.h"
#include <memory>
#include "StaticMesh.h"

class AStaticMeshActor
{
public:
	AStaticMeshActor(std::string InName = "") {
		
		if (InName == "")
		{
			Utilities::Print(L"Error!! Actor should have a name!!\n");
		}

		bActorTransformDirty = false;
		ActorName = new std::string(InName);

		ObjectConstants = std::make_shared<FObjectConstants>();

		Translation = FVector4D(0.0f, 0.0f, 0.0f, 0.0f);
		Rotation = FVector4D(0.0f, 0.0f, 0.0f, 0.0f);
		Scaling = FVector4D(1.0f, 1.0f, 1.0f, 1.0f);
	}

	virtual ~AStaticMeshActor() { 
		ObjectConstants.reset();

	}

	virtual void Tick(float DeltaTime);

	std::string* GetName(){return ActorName;}

	void SetStaticMesh(std::shared_ptr<UStaticMesh> InMesh){ StaticMesh = InMesh;};
	std::weak_ptr<UStaticMesh> GetStaticMesh(){return StaticMesh;};

	FObjectConstants* GetObjectConstants(){return ObjectConstants.get();}

	//called before create rendering mesh
	void InitiallySetLocation(FVector InLoc);

	void InitiallySetRotation(FVector4D InQuat);

	void InitiallySetScaling(FVector InScaling);

	//TODO: Dirty Constant Data
	void SetLocation(FVector NewLoc);
	FVector GetLocation();

	void SetRotation(FVector NewRot);
	FVector GetRotation();

	void SetScale(FVector NewScale);
	FVector GetScale();


protected:
	
	FVector4D Translation;
	FVector4D Rotation;
	FVector4D Scaling;

	std::string* ActorName;
	std::shared_ptr<FObjectConstants> ObjectConstants;

	void MarkActorTransformDirty() { bActorTransformDirty = true; }

	bool bActorTransformDirty;

private:
	std::weak_ptr<UStaticMesh> StaticMesh;
};
