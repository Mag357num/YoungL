#pragma once
#include "StaticMeshActor.h"
#include "../Utilities.h"

class AInstancedStaticMeshActor : public AStaticMeshActor
{
public:
	AInstancedStaticMeshActor(std::string InName = "")
		:AStaticMeshActor(InName)
	{

	}

	virtual ~AInstancedStaticMeshActor()
	{
		if (!Instances.empty())
		{
		}
	}

	virtual void Tick(float DeltaTime)override;

	void AddInstance(FActorInstanceInfo InInstance);
	void UpdateInstance(UINT Index, FActorInstanceInfo InInstance);
	void RemoveInstance(UINT Index);

	//TODO: Dirty Instance Data

private:
	std::vector<FActorInstanceInfo> Instances;
};
