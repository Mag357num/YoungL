#pragma once

#include "Utilities.h"
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

private:
	std::unique_ptr<FObjectConstants> ObjectConstants;
	std::unique_ptr<FGeometry> Geometry;
};
