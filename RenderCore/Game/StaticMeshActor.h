#pragma once

#include "../Utilities.h"
#include "../Math/Math.h"
#include <memory>

class AStaticMeshActor
{
public:
	AStaticMeshActor(std::string InName = "") {
		
		if (InName == "")
		{
			Utilities::Print(L"Error!! Actor should have a name!!\n");
		}

		ActorName = new std::string(InName);

		ObjectConstants = std::make_unique<FObjectConstants>();

		Translation = FVector4D(0.0f, 0.0f, 0.0f, 0.0f);
		Rotation = FVector4D(0.0f, 0.0f, 0.0f, 0.0f);
		Scaling = FVector4D(1.0f, 1.0f, 1.0f, 1.0f);
	}

	virtual ~AStaticMeshActor() { ObjectConstants.reset(); Geometry.reset(); }

	virtual void Tick(float DeltaTime){}

	std::string* GetName(){return ActorName;}

	void SetGeometry(std::unique_ptr<FGeometry<FVertex>>& InGeometry) { Geometry = std::move(InGeometry); };
	FGeometry<FVertex>* GetGeometry(){ return Geometry.get(); }
	FObjectConstants* GetObjectConstants(){return ObjectConstants.get();}

	//called before create rendering mesh
	void InitiallySetLocation(FVector InLoc){
		Translation.X = InLoc.X;
		Translation.Y = InLoc.Y;
		Translation.Z = InLoc.Z;

		ObjectConstants->ObjectWorld = FMath::MatrixAffineTransformation(Scaling, Rotation, Translation);
		ObjectConstants->ObjectWorld = FMath::MatrixTranspose(ObjectConstants->ObjectWorld);
	}

	void InitiallySetRotation(FVector4D InQuat) {
		Rotation.X = InQuat.X;
		Rotation.Y = InQuat.Y;
		Rotation.Z = InQuat.Z;
		Rotation.W = InQuat.W;

		ObjectConstants->ObjectWorld = FMath::MatrixAffineTransformation(Scaling, Rotation, Translation);
		ObjectConstants->ObjectWorld = FMath::MatrixTranspose(ObjectConstants->ObjectWorld);
	}

	void InitiallySetScaling(FVector InScaling) {
		Scaling.X = InScaling.X;
		Scaling.Y = InScaling.Y;
		Scaling.Z = InScaling.Z;

		ObjectConstants->ObjectWorld = FMath::MatrixAffineTransformation(Scaling, Rotation, Translation);
		ObjectConstants->ObjectWorld = FMath::MatrixTranspose(ObjectConstants->ObjectWorld);
	}

protected:
	
	FVector4D Translation;
	FVector4D Rotation;
	FVector4D Scaling;

	std::string* ActorName;
	std::unique_ptr<FObjectConstants> ObjectConstants;

private:
	std::unique_ptr<FGeometry<FVertex>> Geometry;
};
