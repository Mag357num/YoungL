#include "pch.h"
#include "InstancedStaticMeshActor.h"

void AInstancedStaticMeshActor::Tick(float DeltaTime)
{
	AStaticMeshActor::Tick(DeltaTime);

	if (InstanceDataDirty)
	{
		BuildTextureInstanceData();
	}
}

void AInstancedStaticMeshActor::AddInstance(FActorInstanceInfo InInstance)
{
	Instances.push_back(InInstance);

	MarkInstanceDataDirty();
}

void AInstancedStaticMeshActor::UpdateInstance(UINT Index, FActorInstanceInfo InInstance)
{
	Instances[Index].Translation = InInstance.Translation;
	Instances[Index].Rotation = InInstance.Rotation;
	Instances[Index].Scaling = InInstance.Scaling;

	MarkInstanceDataDirty();
}

void AInstancedStaticMeshActor::RemoveInstance(UINT Index)
{
	Instances.erase(Instances.begin() + Index);

	MarkInstanceDataDirty();
}

void AInstancedStaticMeshActor::BuildTextureInstanceData()
{
	InstanceDataDirty = false;

	if (!InstanceTextureData)
	{
		InstanceTextureData = UTexture::CreateTextureWithClear(128, 128, FColorPreset::LightBlue);
	}

	//request update render resource
	InstanceTextureData->RequestUpdateRenderResource();
}

std::shared_ptr<UTexture> AInstancedStaticMeshActor::GetTextureInstanceData()
{
	return InstanceTextureData;
}