#pragma once
#include "StaticMeshActor.h"
#include "SkinData.h"
#include "SkeletalMesh.h"

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
private:
	FSkinedData* SkinedData;

private:

	std::weak_ptr<USkeletalMesh> SkeletalMesh;

	FAnimationPlayInfo* PlayInfo;
	FBoneTransforms* BoneTransforms;
};
