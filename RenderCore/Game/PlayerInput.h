#pragma once
#include "Camera.h"
#include "SkeletalMeshActor.h"

class UPlayerInput
{
public:
	UPlayerInput();
	~UPlayerInput();

	void SetBindedCamera(std::shared_ptr<UCamera> InCamera);
	void SetBindedCharacter();

private:
	std::weak_ptr<UCamera> BindedCamera;
	std::weak_ptr<ASkeletalMeshActor> BindedCharacter;
};
