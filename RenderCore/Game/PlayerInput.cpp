#include "pch.h"
#include "PlayerInput.h"

UPlayerInput::UPlayerInput()
{
}

UPlayerInput::~UPlayerInput()
{
}

void UPlayerInput::SetBindedCamera(std::shared_ptr<UCamera> InCamera)
{
	BindedCamera = InCamera;
}