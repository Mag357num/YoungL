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

	virtual ~AMeshActor() { ObjectConstants.reset(); Geometry.reset(); }

	void SetGeometry(std::unique_ptr<FGeometry<FVertex>>& InGeometry) { Geometry = std::move(InGeometry); };
	FGeometry<FVertex>* GetGeometry(){ return Geometry.get(); }
	FObjectConstants* GetObjectConstants(){return ObjectConstants.get();}

	//called before create rendering mesh
	void InitiallySetLocation(FVector InLoc){
		ObjectConstants->ObjectWorld.Content[3][0] = InLoc.X;
		ObjectConstants->ObjectWorld.Content[3][1] = InLoc.Y;
		ObjectConstants->ObjectWorld.Content[3][2] = InLoc.Z;

		ObjectConstants->ObjectWorld = FMath::MatrixTranspose(ObjectConstants->ObjectWorld);
	}

protected:
	std::unique_ptr<FObjectConstants> ObjectConstants;

private:
	std::unique_ptr<FGeometry<FVertex>> Geometry;
};
