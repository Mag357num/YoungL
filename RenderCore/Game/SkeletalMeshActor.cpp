#include "pch.h"
#include "SkeletalMeshActor.h"

#include "../Engine.h"
#include "../Render/RenderThreadManager.h"

ASkeletalMeshActor::ASkeletalMeshActor(std::string InName)
	:AStaticMeshActor(InName)
{
	SkinedData = new FSkinedData();
	PlayInfo = new FAnimationPlayInfo();

	BoneTransforms = new FBoneTransforms();
}

ASkeletalMeshActor::~ASkeletalMeshActor()
{
	if (SkinedData)
	{
		delete SkinedData;
		SkinedData = nullptr;
	}

	if (PlayInfo)
	{
		delete PlayInfo;
		PlayInfo = nullptr;
	}

	if (BoneTransforms)
	{
		delete BoneTransforms;
		BoneTransforms = nullptr;
	}
}

void ASkeletalMeshActor::TestPlayAnimation()
{
	//PlayAnimationClip("Idle");
	PlayAnimationClip("Walk");
}

void ASkeletalMeshActor::PlayAnimationClip(std::string ClipName)
{
	PlayInfo->ClipName = ClipName;
	PlayInfo->PlayedTime = 0.0f;

	PlayInfo->RequestPlaying = true;
	PlayInfo->IsPlaying = false;

	PlayInfo->ClipEndTime = SkinedData->GetClipEndTime(ClipName);
}

void ASkeletalMeshActor::StopAnimationClip()
{
	PlayInfo->RequestPlaying = false;
	PlayInfo->IsPlaying = false;

	PlayInfo->PlayedTime = 0.0f;
}

static void UpdateSkinnedMeshBoneTransform_RenderThread(std::string* ActorName, FBoneTransforms* InBoneTrans)
{
	if (InBoneTrans && ActorName)
	{
		FRenderThreadManager::UpdateSkinnedMeshBoneTransform(ActorName, InBoneTrans);
	}
	
}

void ASkeletalMeshActor::Tick(float DeltaTime)
{
	AStaticMeshActor::Tick(DeltaTime);

	if (SkinedData)
	{
		//start to play
		if (PlayInfo->RequestPlaying)
		{
			PlayInfo->RequestPlaying = false;
			PlayInfo->IsPlaying = true;
			PlayInfo->PlayedTime += DeltaTime;
		}

		if (PlayInfo->IsPlaying)
		{
			//lerp key frames using played time
			SkinedData->CalcBoneTransormsByTime(PlayInfo->ClipName, PlayInfo->PlayedTime, BoneTransforms);
			PlayInfo->PlayedTime += DeltaTime;

			//loop annimation
			if (PlayInfo->PlayedTime > PlayInfo->ClipEndTime)
			{
				PlayInfo->PlayedTime = PlayInfo->PlayedTime - PlayInfo->ClipEndTime;
			}

			//update bone transform buffer
			FRenderThreadCommand UpdateBoneTransfomrsCommand;

			UpdateBoneTransfomrsCommand.Wrap(UpdateSkinnedMeshBoneTransform_RenderThread, ActorName, BoneTransforms);
			FEngine::GetEngine()->GetRenderThreadManager()->PushRenderCommand(UpdateBoneTransfomrsCommand);
		}
	}
}