#include "pch.h"
#include "InstancedStaticMeshActor.h"

void AInstancedStaticMeshActor::Tick(float DeltaTime)
{
	AStaticMeshActor::Tick(DeltaTime);
}

void AInstancedStaticMeshActor::AddInstance(FActorInstanceInfo InInstance)
{
	Instances.push_back(InInstance);
}

void AInstancedStaticMeshActor::UpdateInstance(UINT Index, FActorInstanceInfo InInstance)
{
	Instances[Index].Translation = InInstance.Translation;
	Instances[Index].Rotation = InInstance.Rotation;
	Instances[Index].Scaling = InInstance.Scaling;
}

void AInstancedStaticMeshActor::RemoveInstance(UINT Index)
{
	Instances.erase(Instances.begin() + Index);
}