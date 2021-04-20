#pragma once
#include <vector>
#include <unordered_map>
#include "../Math/Math.h"


struct FBoneKeyFrame
{
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

private:
	std::vector<uint8_t> BoneHierarchy;
	std::vector<FMatrix> BoneOffset;

	std::vector<std::string> BoneNames;

	std::unordered_map<std::string, FAnimationClip> Animations;
};
