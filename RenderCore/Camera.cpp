#include "Camera.h"

#define  CAMERAPI 3.1416f

FCamera::FCamera()
	:CamPos(FVector4D(800.0f, 0.0f, 150.0f, 0.0f)),
	CamTarget(FVector(0.0f, 0.0f, 150.0f)),
	CamUp(FVector(0.0f, 0.0f, 1.0f)),
	NearPlan(1.0f),
	FarPlan(1000.0f),
	Fov(45.0f),
	AspectRatio(1.333f)//800x600

{
	FVector4D TempCameraTarget = FVector4D(CamTarget.X, CamTarget.Y, CamTarget.Z, 0.0f);
	FVector4D TempCameraLoc = FVector4D(CamPos.X, CamPos.Y, CamPos.Z, 0.0f);
	FVector4D TempUp = FVector4D(CamUp.X, CamUp.Y, CamUp.Z, 0.0f);
	FVector4D LengthRe = Utilities::Vector3Length(Utilities::VectorSubtract(TempCameraTarget, TempCameraLoc));
	
	CamRadius = LengthRe.X;
	CamPitch = 0.0f;
	CamRotate = 0.0f;

	IsCamerInfoDirty = false;

	View = Utilities::MatrixLookAtLH(TempCameraLoc, TempCameraTarget, TempUp);
	Proj = Utilities::MatrixPerspectiveFovLH(CAMERAPI * Fov/180.0f, AspectRatio, 1.0f, 1000.0f);

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
	Proj = Utilities::MatrixPerspectiveFovLH(CAMERAPI * Fov / 180.0f, AspectRatio, 1.0f, 1000.0f);
	IsCamerInfoDirty = true;
}

void FCamera::Pitch(float Dy)
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

void FCamera::Rotate(float Dx)
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