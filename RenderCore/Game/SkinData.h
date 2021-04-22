#pragma once
#include <vector>
#include <unordered_map>
#include "../Math/Math.h"


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
	uint8_t BoneIndex;
	std::vector<FBoneKeyFrame> KeyFrames;//mapped with key frames of a bone point
};

struct FAnimationClip
{
	std::vector<FBoneAnimation> BoneAnimations;//mapped with bones
};

class FSkinedData
{
public:
	FSkinedData(){}
	~FSkinedData(){}


	void SetData(std::vector<int>& InHierarchy, std::vector<FMatrix> InBoneOffset,
		::unordered_map<std::string, FAnimationClip>& InAnimations)
	{
		BoneHierarchy = InHierarchy;
		BoneOffset = InBoneOffset;
		Animations = InAnimations;
	}

private:
	std::vector<int> BoneHierarchy;
	std::vector<FMatrix> BoneOffset;

	std::vector<std::string> BoneNames;

	std::unordered_map<std::string, FAnimationClip> Animations;
};
