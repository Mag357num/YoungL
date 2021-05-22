#pragma once
#include "StaticMeshActor.h"
#include "SkinData.h"
#include "SkeletalMesh.h"

enum EAnimationState
{
	State_Idle,
	State_Walk
};


class ASkeletalMeshActor : public AStaticMeshActor
{
public:
	ASkeletalMeshActor(std::string InName = "");

	virtual ~ASkeletalMeshActor();

	FSkinedData* GetSkinedData(){return SkinedData;}

	virtual void Tick(float DeltaTime)override;

	void SetSkeletalMesh(std::shared_ptr<USkeletalMesh> InMesh) { SkeletalMesh = InMesh; };
	std::weak_ptr<USkeletalMesh> GetSkeletalMesh() { return SkeletalMesh; };

	FBoneTransforms* GetBoneTransfroms(){return BoneTransforms;}

	void TestPlayAnimation();

	void PlayAnimationClip(std::string ClipName);
	void StopAnimationClip();


	EAnimationState GetAnimState(){return AniState;}
	void SetAniState(EAnimationState InState){
		if (InState != AniState)
		{
			bAniStateDirty = true;
			AniState = InState;
		}
	}

private:
	FSkinedData* SkinedData;

private:
	void TickAniState(float DeltaTime);

	EAnimationState AniState;
	bool bAniStateDirty;

	std::weak_ptr<USkeletalMesh> SkeletalMesh;

	FAnimationPlayInfo* PlayInfo;
	FBoneTransforms* BoneTransforms;
};
