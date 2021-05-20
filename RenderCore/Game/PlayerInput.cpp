#include "pch.h"
#include "PlayerInput.h"

UPlayerInput::UPlayerInput()
{
	bMouseButtonDown = false;
	MousePosition = FVector2D(0.0f, 0.0f);


	std::shared_ptr<FInputBinding> WBinding = std::make_shared<FInputBinding>();
	WBinding->Bind(std::bind(&UPlayerInput::OnWPressed, this, std::placeholders::_1));
	InputBindings.insert(std::make_pair(EDigitalInput::Key_W, WBinding));//VK_LEFT
}

UPlayerInput::~UPlayerInput()
{
	//clear binding
	InputBindings.clear();
}

void UPlayerInput::SetBindedCamera(std::shared_ptr<UCamera> InCamera)
{
	BindedCamera = InCamera;
}


void UPlayerInput::SetBindedCharacter(std::shared_ptr<ASkeletalMeshActor> InCharacter)
{
	BindedCharacter = InCharacter;

	BindCameraToCharacter();
}

void UPlayerInput::Tick(float DeltaTime)
{
	if (bMouseButtonDown)
	{
		POINT CurPoint = { 0, 0 };
		GetCursorPos(&CurPoint);
		OnMouseMove(WM_LBUTTONDOWN, CurPoint.x - WindowOffset.x, CurPoint.y - WindowOffset.y);
	}

	for (auto It = InputBindings.begin(); It != InputBindings.end(); ++It)
	{
		if (It->second->GetIsActive())
		{
			It->second->Excute(DeltaTime);
		}
	}
}


void UPlayerInput::OnKeyDown(UINT8 Key)
{
	//std::string OutputStr = std::to_string(Key);
	//OutputStr = "*** On Key Pressed " + OutputStr;
	//Utilities::Print(OutputStr.c_str());
	//Utilities::Print("\n");

	std::map<UINT8, std::shared_ptr<FInputBinding>>::iterator Iter;
	Iter = InputBindings.find(Key);
	if (Iter != InputBindings.end())
	{
		Iter->second->SetIsActive(true);
	}
}

void UPlayerInput::OnKeyUp(UINT8 Key)
{
	std::map<UINT8, std::shared_ptr<FInputBinding>>::iterator Iter;
	Iter = InputBindings.find(Key);
	if (Iter != InputBindings.end())
	{
		Iter->second->SetIsActive(false);
	}
}

void UPlayerInput::OnMouseButtonDown(WPARAM BtnState, int X, int Y)
{
	MousePosition.X = (float)X;
	MousePosition.Y = (float)Y;

	POINT CurPoint = { 0, 0 };
	GetCursorPos(&CurPoint);
	WindowOffset.x = CurPoint.x - X;
	WindowOffset.y = CurPoint.y - Y;

	bMouseButtonDown = true;
}

void UPlayerInput::OnMouseButtonUp(WPARAM BtnState, int X, int Y)
{
	bMouseButtonDown = false;
}

void UPlayerInput::OnMouseMove(WPARAM BtnState, int X, int Y)
{
	if (bMouseButtonDown)
	{
		//get window offset
		float Dx = X - MousePosition.X;
		float Dy = Y - MousePosition.Y;

		BindedCamera.lock()->Pitch(Dy * 0.2f);
		BindedCamera.lock()->Rotate(Dx * 0.2f);


		MousePosition.X = (float)X;
		MousePosition.Y = (float)Y;
	}
}


void UPlayerInput::BindCameraToCharacter()
{

}

void UPlayerInput::OnWPressed(float DeltaTime)
{
	//Utilities::Print(L"*** On W Pressed ***\n");
	FVector CharacterPos = BindedCharacter.lock()->GetLocation();
	FVector ForwarDir = BindedCamera.lock()->GetForwardDirection();
	ForwarDir.Z = 0.0f;//reduce z move to 0
	ForwarDir = ForwarDir.Normalize();

	FVector TargetLoc = CharacterPos + ForwarDir * (DeltaTime * 100.0f);
	BindedCharacter.lock()->SetLocation(TargetLoc);
}