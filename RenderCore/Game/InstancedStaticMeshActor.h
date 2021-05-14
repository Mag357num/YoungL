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

		if (InstanceTextureData)
		{
			InstanceTextureData.reset();
		}
	}

	virtual void Tick(float DeltaTime)override;

	void AddInstance(FActorInstanceInfo InInstance);
	void UpdateInstance(UINT Index, FActorInstanceInfo InInstance);
	void RemoveInstance(UINT Index);

	std::shared_ptr<UTexture> GetTextureInstanceData();
	size_t GetInstanceCount(){return Instances.size();}

private:
	void MarkInstanceDataDirty(){ InstanceDataDirty = true;}
	bool InstanceDataDirty;
	void BuildTextureInstanceData();

	std::vector<FActorInstanceInfo> Instances;
	std::shared_ptr<UTexture> InstanceTextureData;
};
