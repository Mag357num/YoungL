#pragma once
#include <functional>

//VK_LEFT
enum EDigitalInput
{
	// keyboard
	// kKey must start at zero, see S_DXKeyMapping

	Key_A = 65,
	Key_D = 68,
	Key_S = 83,
	Key_W = 87,

	kNumDigitalInputs
};

class FInputBinding
{
public:
	FInputBinding()
	{
		IsActive = false;
	}

	~FInputBinding()
	{

	}

	void Excute(float DeltaTime)
	{
		OnKeyPressed(DeltaTime);
	}

	void Bind(std::function<void(float)> InFunc)
	{
		OnKeyPressed = InFunc;
	}

	void SetIsActive(bool InFlag){IsActive = InFlag;}
	bool GetIsActive(){return IsActive;}

private:
	std::function<void(float)> OnKeyPressed;
	bool IsActive;
};

