#pragma once
#include <vector>
#include <unordered_map>
#include "../Math/Math.h"

struct FAnimationPlayInfo
{
	FAnimationPlayInfo(){}

	std::string ClipName;
	float StartTime;
	float PlayedTime;

	bool RequestPlaying;
	bool IsPlaying;

	float ClipEndTime;
};

struct FBoneKeyFrame
{

	float TimePos;
	int FrameIndex;
	//save transform of one bone point in a key frame
	FVector4D Translation;
	FVector4D Rotate;
	FVector4D Scale;
};

struct FBoneAnimation
{
	void Interpolate(float PlayedTime, FMatrix& OutTransform)
	{

		if (PlayedTime < KeyFrames.front().TimePos)
		{
			FVector4D Translation = KeyFrames.front().Translation;
			FVector4D QuadRotation = KeyFrames.front().Rotate;
			FVector4D Scaling = KeyFrames.front().Scale;

			OutTransform = FMath::MatrixAffineTransformation_QuadRot(Scaling, QuadRotation, Translation);
		}
		else if (PlayedTime > KeyFrames.back().TimePos)
		{
			FVector4D Translation = KeyFrames.back().Translation;
			FVector4D QuadRotation = KeyFrames.back().Rotate;
			FVector4D Scaling = KeyFrames.back().Scale;

			OutTransform = FMath::MatrixAffineTransformation_QuadRot(Scaling, QuadRotation, Translation);
		}
		else
		{
			for (UINT i = 0; i < KeyFrames.size() - 1; i++)
			{
				if (PlayedTime > KeyFrames[i].TimePos && PlayedTime < KeyFrames[i+1].TimePos)
				{
					float LerpPercent = (PlayedTime - KeyFrames[i].TimePos) / (KeyFrames[i + 1].TimePos - KeyFrames[i].TimePos);

					FVector4D Translation0 = KeyFrames[i].Translation;
					FVector4D QuadRotation0 = KeyFrames[i].Rotate;
					FVector4D Scaling0 = KeyFrames[i].Scale;

					FVector4D Translation1 = KeyFrames[i + 1].Translation;
					FVector4D QuadRotation1 = KeyFrames[i + 1].Rotate;
					FVector4D Scaling1 = KeyFrames[i + 1].Scale;

					FVector4D Translation = FMath::VectorLerp(Translation0, Translation1, LerpPercent);
					FVector4D Scaling = FMath::VectorLerp(Scaling0, Scaling1, LerpPercent);

					//lerp quaternion; linear lerp
					//todo: sphere lerp
					FVector4D QuadRotation = FMath::VectorLerp(QuadRotation0, QuadRotation1, LerpPercent);

					OutTransform = FMath::MatrixAffineTransformation_QuadRot(Scaling, QuadRotation, Translation);

				}
			}
		}
	}


	float GetEndTime()
	{
		return KeyFrames.back().TimePos;
	}

	uint8_t BoneIndex;
	std::vector<FBoneKeyFrame> KeyFrames;//mapped with key frames of a bone point
};

struct FAnimationClip
{
	void Interpolate(float PlayedTime, std::vector<FMatrix>& OutBoneTransforms)
	{
		for (UINT i = 0; i < BoneAnimations.size(); ++i)
		{
			BoneAnimations[i].Interpolate(PlayedTime, OutBoneTransforms[i]);
		}
	}

	float GetClipEndTime()
	{
		float ClipEndTime = 0.0f;
		for (int BoneIndex = 0; BoneIndex < BoneAnimations.size(); ++BoneIndex)
		{
			if (BoneAnimations[BoneIndex].GetEndTime() > ClipEndTime)
			{
				ClipEndTime = BoneAnimations[BoneIndex].GetEndTime();
			}
		}

		return ClipEndTime;
	}

	std::vector<FBoneAnimation> BoneAnimations;//mapped with bones
};

class FSkinedData
{
public:
	FSkinedData(){}
	~FSkinedData(){}

	float GetClipEndTime(std::string ClipName)
	{
		auto clip = Animations.find(ClipName);
		return clip->second.GetClipEndTime();
	}

	void SetData(std::vector<int>& InHierarchy, std::vector<FMatrix> InBoneOffset,
		::unordered_map<std::string, FAnimationClip>& InAnimations)
	{
		BoneHierarchy = InHierarchy;
		BoneOffset = InBoneOffset;
		Animations = InAnimations;
	}

	void CalcBoneTransormsByTime(std::string ClipName, float PlayedTime, FBoneTransforms* OutBoneTransforms)
	{
		//search playing frame
		FAnimationClip* PlayingClip = &Animations[ClipName];

		UINT NumBones = (UINT)BoneOffset.size();
		std::vector<FMatrix> ToParentTransforms;
		ToParentTransforms.resize(NumBones);

		//lerp animation , get toparent transforms
		PlayingClip->Interpolate(PlayedTime, ToParentTransforms);

		//calc final transform
		std::vector<FMatrix> ToRootTransforms;
		ToRootTransforms.resize(NumBones);

		//0: is root bone
		ToRootTransforms[0] = ToParentTransforms[0];
		for (UINT BoneIndex = 1; BoneIndex < NumBones; BoneIndex++)
		{
			FMatrix ToParent = ToParentTransforms[BoneIndex];

			int ParentIndex = BoneHierarchy[BoneIndex];
			FMatrix ParentToRoot = ToRootTransforms[ParentIndex];
			FMatrix ToRoot = ToParent * ParentToRoot;

			ToRootTransforms[BoneIndex] = ToRoot;
		}
		
		//multiply offset in bone point space
		for (UINT BoneIndex = 0; BoneIndex < NumBones; BoneIndex++)
		{
			FMatrix Offset = BoneOffset[BoneIndex];
			FMatrix ToRoot = ToRootTransforms[BoneIndex];
			FMatrix FinalMatrix = Offset * ToRoot;

			FinalMatrix = FMath::MatrixTranspose(FinalMatrix);
			OutBoneTransforms->BoneTransforms[BoneIndex] = FinalMatrix;
		}
	}

private:
	//
	std::vector<int> BoneHierarchy;
	std::vector<FMatrix> BoneOffset;//offset in bone point space

	std::vector<std::string> BoneNames;

	std::unordered_map<std::string, FAnimationClip> Animations;
};
