#pragma once

#include "../Utilities.h"
#include "../Math/Math.h"
#include <memory>

class AMeshActor
{
public:
	AMeshActor() {
		
		ObjectConstants = std::make_unique<FObjectConstants>();
	}

	~AMeshActor() { ObjectConstants.release(); Geometry.release(); }

	void SetGeometry(std::unique_ptr<FGeometry>& InGeometry) { Geometry = std::move(InGeometry); };
	FGeometry* GetGeometry(){ return Geometry.get(); }
	FObjectConstants* GetObjectConstants(){return ObjectConstants.get();}

	void SetLocation(FVector InLoc){
		ObjectConstants->ObjectWorld.Content[3][0] = InLoc.X;
		ObjectConstants->ObjectWorld.Content[3][1] = InLoc.Y;
		ObjectConstants->ObjectWorld.Content[3][2] = InLoc.Z;

		ObjectConstants->ObjectWorld = FMath::MatrixTranspose(ObjectConstants->ObjectWorld);
	}

private:
	std::unique_ptr<FObjectConstants> ObjectConstants;
	std::unique_ptr<FGeometry> Geometry;
};
