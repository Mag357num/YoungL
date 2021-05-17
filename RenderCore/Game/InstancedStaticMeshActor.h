#pragma once
#include "StaticMeshActor.h"
#include "../Utilities.h"
#include "Texture.h"

class AInstancedStaticMeshActor : public AStaticMeshActor
{
public:
	AInstancedStaticMeshActor(std::string InName = "")
		:AStaticMeshActor(InName)
	{
		InstanceDataDirty = false;
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

	size_t GetInstanceCount(){return Instances.size();}

	std::vector<FInstanceData> CalcInstanceDatas();

private:
	void MarkInstanceDataDirty(){ InstanceDataDirty = true;}
	bool InstanceDataDirty;

	std::vector<FActorInstanceInfo> Instances;
};
