#pragma once
#include "Camera.h"
#include "SkeletalMeshActor.h"
#include "InputBinding.h"

#include <map>

class UPlayerInput
{
public:
	UPlayerInput();
	~UPlayerInput();

	void SetBindedCamera(std::shared_ptr<UCamera> InCamera);
	//called after binded camera is set
	void SetBindedCharacter(std::shared_ptr<ASkeletalMeshActor> InCharacter);

	virtual void Tick(float DeltaTime);

	virtual void OnKeyDown(UINT8 Key);
	virtual void OnKeyUp(UINT8 Key);

	virtual void OnMouseButtonDown(WPARAM BtnState, int X, int Y);
	virtual void OnMouseButtonUp(WPARAM BtnState, int X, int Y);
	virtual void OnMouseMove(WPARAM BtnState, int X, int Y);


private:

	void BindCameraToCharacter();

	void OnWPressed(float DeltaTime);

	
	std::weak_ptr<UCamera> BindedCamera;
	std::weak_ptr<ASkeletalMeshActor> BindedCharacter;

	//mouse position
	FVector2D MousePosition;
	//saved to update delta x && delta Y in window
	POINT WindowOffset;

	bool bMouseButtonDown;


	//bindings
	std::map<UINT8, std::shared_ptr<FInputBinding>> InputBindings;
};
