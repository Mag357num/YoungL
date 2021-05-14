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

static void UpdateInstanceTextureData_RenderThread(std::string* ActorName, std::shared_ptr<UTexture> InstanceData)
{
	FRenderThreadManager::UpdateInstanceTextureData(ActorName, InstanceData.get());
}

void AInstancedStaticMeshActor::BuildTextureInstanceData()
{
	InstanceDataDirty = false;

	if (!InstanceTextureData)
	{
		InstanceTextureData = UTexture::CreateTextureWithClear(128, 128, FColorPreset::LightBlue);
	}

	//request update render resource
	InstanceTextureData->RequestUpdateRenderResource(Instances);
	//request upload texture data
	FRenderThreadManager* RenderThreadManager = FEngine::GetEngine()->GetRenderThreadManager();

	FRenderThreadCommand UpdateInstanceDataCommand;
	UpdateInstanceDataCommand.Wrap(UpdateInstanceTextureData_RenderThread, ActorName, InstanceTextureData);
	RenderThreadManager->PushRenderCommand(UpdateInstanceDataCommand);
}

std::shared_ptr<UTexture> AInstancedStaticMeshActor::GetTextureInstanceData()
{
	return InstanceTextureData;
}