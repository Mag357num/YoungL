#include "pch.h"
#include "Camera.h"

#define  CAMERAPI 3.1416f

UCamera::UCamera(int ViewWidth, int ViewHeigt)
	:CamPos(FVector4D(800.0f, 0.0f, 150.0f, 0.0f)),
	CamTarget(FVector(0.0f, 0.0f, 150.0f)),
	CamUp(FVector(0.0f, 0.0f, 1.0f)),
	NearPlan(1.0f),
	FarPlan(10000.0f),
	Fov(45.0f),
	AspectRatio(1.333f)//800x600

{
	AspectRatio = 1.0f*ViewWidth / ViewHeigt;

	FVector4D TempCameraTarget = FVector4D(CamTarget.X, CamTarget.Y, CamTarget.Z, 0.0f);
	FVector4D TempCameraLoc = FVector4D(CamPos.X, CamPos.Y, CamPos.Z, 0.0f);
	FVector4D TempUp = FVector4D(CamUp.X, CamUp.Y, CamUp.Z, 0.0f);
	FVector4D LengthRe = FMath::Vector3Length(FMath::VectorSubtract(TempCameraTarget, TempCameraLoc));
	
	CamRadius = LengthRe.X;
	CamPitch = 0.0f;
	CamRotate = 0.0f;

	IsCamerInfoDirty = false;

	View = FMath::MatrixLookAtLH(TempCameraLoc, TempCameraTarget, TempUp);
	Proj = FMath::MatrixPerspectiveFovLH(CAMERAPI * Fov/180.0f, AspectRatio, NearPlan, FarPlan);

	IsCamerInfoDirty = true;
}

UCamera::~UCamera()
{

}

void UCamera::UpdateView()
{
	FVector4D TempCameraTarget = FVector4D(CamTarget.X, CamTarget.Y, CamTarget.Z, 0.0f);
	FVector4D TempCameraLoc = FVector4D(CamPos.X, CamPos.Y, CamPos.Z, 0.0f);
	FVector4D TempUp = FVector4D(CamUp.X, CamUp.Y, CamUp.Z, 0.0f);

	View = FMath::MatrixLookAtLH(TempCameraLoc, TempCameraTarget, TempUp);

	IsCamerInfoDirty = true;
}

void UCamera::UpdateProj()
{
	Proj = FMath::MatrixPerspectiveFovLH(CAMERAPI * Fov / 180.0f, AspectRatio, NearPlan, FarPlan);
	IsCamerInfoDirty = true;
}

void UCamera::Pitch(float Dy)
{
	float Radians = CAMERAPI * Dy/180.0f;
	CamPitch += Radians;

	if (CamPitch >= CAMERAPI || CamPitch <= -CAMERAPI)
	{
		return;
	}

	float LocZ = CamRadius * (float)sin(CamPitch);
	float LocX = CamRadius * (float)cos(CamPitch) * (float)cos(CamRotate);
	float LocY = CamRadius * (float)cos(CamPitch) * (float)sin(CamRotate);

	CamPos.X = CamTarget.X + LocX;
	CamPos.Y = CamTarget.Y + LocY;
	CamPos.Z = CamTarget.Z + LocZ;

	UpdateView();
}

void UCamera::Rotate(float Dx)
{
	float Radians = CAMERAPI * Dx / 180.0f;
	CamRotate += Radians;

	float LocZ = CamRadius * (float)sin(CamPitch);
	float LocX = CamRadius * (float)cos(CamPitch) * (float)cos(CamRotate);
	float LocY = CamRadius * (float)cos(CamPitch) * (float)sin(CamRotate);

	CamPos.X = CamTarget.X + LocX;
	CamPos.Y = CamTarget.Y + LocY;
	CamPos.Z = CamTarget.Z + LocZ;

	UpdateView();
}

FVector UCamera::GetForwardDirection()
{
	FVector Ret( CamTarget.X - CamPos.X, CamTarget.Y - CamPos.Y, CamTarget.Z - CamPos.Z);
	return Ret.Normalize();
}

//FVector UCamera::GetLeftDirection()
//{
//	FVector Ret(CamTarget.X - CamPos.X, CamTarget.Y - CamPos.Y, CamTarget.Z - CamPos.Z);
//
//	return Ret;
//}

void UCamera::SetCameraLocation(FVector InNewLoc)
{

}

void UCamera::SetCameraTargetLoc(FVector InNewTarget)
{

}

void UCamera::SetFov(float InFov)
{

}

void UCamera::SetAspectRatio(float InAspectRatio)
{

}