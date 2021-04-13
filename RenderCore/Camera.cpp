#include "Camera.h"

FCamera::FCamera()
	:CamPos(FVector4D(500.0f, 500.0f, 100.0f, 0.0f)),
	CamTarget(FVector(0.0f, 0.0f, 150.0f)),
	CamUp(FVector(0.0f, 0.0f, 1.0f)),
	NearPlan(1.0f),
	FarPlan(1000.0f),
	Fov(45.0f),
	AspectRatio(1.78f)//1280x720

{
	FVector4D TempCameraTarget = FVector4D(CamTarget.X, CamTarget.Y, CamTarget.Z, 0.0f);
	FVector4D TempCameraLoc = FVector4D(CamPos.X, CamPos.Y, CamPos.Z, 0.0f);
	FVector4D TempUp = FVector4D(CamUp.X, CamUp.Y, CamUp.Z, 0.0f);
	FVector4D LengthRe = Utilities::Vector3Length(Utilities::VectorSubtract(TempCameraTarget, TempCameraLoc));
	Radius = LengthRe.X;

	IsCamerInfoDirty = false;

	View = Utilities::MatrixLookAtLH(TempCameraLoc, TempCameraTarget, TempUp);
	Proj = Utilities::MatrixPerspectiveFovLH(3.1416f * Fov/180.0f, AspectRatio, 1.0f, 1000.0f);

	IsCamerInfoDirty = true;
}

FCamera::~FCamera()
{

}

void FCamera::UpdateView()
{
	FVector4D TempCameraTarget = FVector4D(CamTarget.X, CamTarget.Y, CamTarget.Z, 0.0f);
	FVector4D TempCameraLoc = FVector4D(CamPos.X, CamPos.Y, CamPos.Z, 0.0f);
	FVector4D TempUp = FVector4D(CamUp.X, CamUp.Y, CamUp.Z, 0.0f);

	View = Utilities::MatrixLookAtLH(TempCameraLoc, TempCameraTarget, TempUp);

	IsCamerInfoDirty = true;
}

void FCamera::UpdateProj()
{
	Proj = Utilities::MatrixPerspectiveFovLH(3.1416f * Fov / 180.0f, AspectRatio, 1.0f, 1000.0f);
	IsCamerInfoDirty = true;
}

void FCamera::Pitch(float Dy)
{

}

void FCamera::Rotate(float Dx)
{

}

void FCamera::SetCameraLocation(FVector InNewLoc)
{

}

void FCamera::SetCameraTargetLoc(FVector InNewTarget)
{

}

void FCamera::SetFov(float InFov)
{

}

void FCamera::SetAspectRatio(float InAspectRatio)
{

}