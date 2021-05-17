#include "pch.h"
#include "InstancedStaticMeshActor.h"

void AInstancedStaticMeshActor::Tick(float DeltaTime)
{
	AStaticMeshActor::Tick(DeltaTime);
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

std::vector<FInstanceData> AInstancedStaticMeshActor::CalcInstanceDatas()
{
	std::vector<FInstanceData> RetArray;
	for (size_t Index = 0; Index < Instances.size(); ++Index)
	{
		FMatrix Transform = FMath::MatrixAffineTransformation(Instances[Index].Scaling, Instances[Index].Rotation, Instances[Index].Translation);
		FInstanceData InstanceData;
		InstanceData.TranslationRow0 = Transform.GetRow0();
		InstanceData.TranslationRow1 = Transform.GetRow1();
		InstanceData.TranslationRow2 = Transform.GetRow2();
		InstanceData.TranslationRow3 = Transform.GetRow3();
		RetArray.push_back(InstanceData);
	}

	return RetArray;
}
