#include "pch.h"
#include "StaticMeshActor.h"


static void UpdateMeshActorTransform_RenderThread(std::string* ActorName, std::shared_ptr<FObjectConstants> ObjectTransform)
{
	if (ActorName)
	{
		FRenderThreadManager::UpdateActorConstantBuffer(*ActorName, ObjectTransform.get());
	}

}

void AStaticMeshActor::Tick(float DeltaTime)
{
	//if tranform dirty
	if (bActorTransformDirty)
	{
		//Utilities::Print("*** bActorTransformDirty true**\n");
		bActorTransformDirty = false;
		ObjectConstants->ObjectWorld = FMath::MatrixAffineTransformation(Scaling, Rotation, Translation);
		ObjectConstants->ObjectWorld = FMath::MatrixTranspose(ObjectConstants->ObjectWorld);

		//notify to render thread to update rendering transform
		//update bone transform buffer
		FRenderThreadCommand UpdateActorTransfomrsCommand;

		UpdateActorTransfomrsCommand.Wrap(UpdateMeshActorTransform_RenderThread, ActorName, ObjectConstants);
		FEngine::GetEngine()->GetRenderThreadManager()->PushRenderCommand(UpdateActorTransfomrsCommand);
	}
}

//called before create rendering mesh
void AStaticMeshActor::InitiallySetLocation(FVector InLoc) {
	Translation.X = InLoc.X;
	Translation.Y = InLoc.Y;
	Translation.Z = InLoc.Z;

	ObjectConstants->ObjectWorld = FMath::MatrixAffineTransformation(Scaling, Rotation, Translation);
	ObjectConstants->ObjectWorld = FMath::MatrixTranspose(ObjectConstants->ObjectWorld);
}

void AStaticMeshActor::InitiallySetRotation(FVector4D InQuat) {
	Rotation.X = InQuat.X;
	Rotation.Y = InQuat.Y;
	Rotation.Z = InQuat.Z;
	Rotation.W = InQuat.W;

	ObjectConstants->ObjectWorld = FMath::MatrixAffineTransformation(Scaling, Rotation, Translation);
	ObjectConstants->ObjectWorld = FMath::MatrixTranspose(ObjectConstants->ObjectWorld);
}

void AStaticMeshActor::InitiallySetScaling(FVector InScaling) {
	Scaling.X = InScaling.X;
	Scaling.Y = InScaling.Y;
	Scaling.Z = InScaling.Z;

	ObjectConstants->ObjectWorld = FMath::MatrixAffineTransformation(Scaling, Rotation, Translation);
	ObjectConstants->ObjectWorld = FMath::MatrixTranspose(ObjectConstants->ObjectWorld);
}

//TODO: Dirty Constant Data
void AStaticMeshActor::SetLocation(FVector NewLoc)
{
	Translation.X = NewLoc.X;
	Translation.Y = NewLoc.Y;
	Translation.Z = NewLoc.Z;

	MarkActorTransformDirty();
}

FVector AStaticMeshActor::GetLocation()
{
	return FVector(Translation.X, Translation.Y, Translation.Z);
}

void AStaticMeshActor::SetRotation(FVector NewRot)
{
	Rotation.X = NewRot.X;
	Rotation.Y = NewRot.Y;
	Rotation.Z = NewRot.Z;
	MarkActorTransformDirty();
}

FVector AStaticMeshActor::GetRotation()
{
	return FVector(Rotation.X, Rotation.Y, Rotation.Z);
}


void AStaticMeshActor::SetScale(FVector NewScale)
{
	Scaling.X = NewScale.X;
	Scaling.Y = NewScale.Y;
	Scaling.Z = NewScale.Z;
	MarkActorTransformDirty();
}

FVector AStaticMeshActor::GetScale()
{
	return FVector(Scaling.X, Scaling.Y, Scaling.Z);
}
