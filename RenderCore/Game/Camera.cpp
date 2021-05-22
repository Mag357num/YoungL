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

void UCamera::RecalcAngles()
{
	FVector4D TempCameraTarget = FVector4D(CamTarget.X, CamTarget.Y, CamTarget.Z, 0.0f);
	FVector4D TempCameraLoc = FVector4D(CamPos.X, CamPos.Y, CamPos.Z, 0.0f);
	FVector4D Delta = FMath::VectorSubtract(TempCameraTarget, TempCameraLoc);
	FVector4D LengthRe = FMath::Vector3Length(Delta);

	CamRadius = LengthRe.X;
	RecalcRotAndPitch();
}

FVector UCamera::GetForwardDirection()
{
	FVector Ret( CamTarget.X - CamPos.X, CamTarget.Y - CamPos.Y, CamTarget.Z - CamPos.Z);
	return Ret.Normalize();
}

FVector UCamera::GetLeftDirection()
{
	double Degree = CamRotate + CAMERAPI * 0.5;


	FVector Ret((float)cos(Degree), (float)sin(Degree), 0);

	return Ret;
}

void UCamera::SetCameraLocation(FVector InNewLoc)
{
	CamPos.X = InNewLoc.X;
	CamPos.Y = InNewLoc.Y;
	CamPos.Z = InNewLoc.Z;

	RecalcRotAndPitch();

	UpdateView();
}

void UCamera::SetCameraTargetLoc(FVector InNewTarget)
{
	CamTarget.X = InNewTarget.X;
	CamTarget.Y = InNewTarget.Y;
	CamTarget.Z = InNewTarget.Z;

	RecalcRotAndPitch();

	UpdateView();
}

void UCamera::RecalcRotAndPitch()
{
	FVector Distance = FVector(CamTarget.X - CamPos.X, CamTarget.Y - CamPos.Y, CamTarget.Z - CamPos.Z);
	FVector Direction = Distance.Normalize();
	float Sqaure = Direction.X * Direction.X + Direction.Y * Direction.Y;
	float SqrtXY = sqrtf(Sqaure);
	float SineRotateF = Direction.Y / SqrtXY;

	if (Direction.X > 0.0f && Direction.Y > 0.0f)
	{
		CamRotate = asinf(SineRotateF);
	}
	else if (Direction.X < 0.0f && Direction.Y > 0.0f)
	{
		CamRotate = CAMERAPI - asinf(SineRotateF);
	}
	else if (Direction.X < 0.0f && Direction.Y < 0.0f)
	{
		CamRotate = CAMERAPI + asinf(SineRotateF);
	}
	else
	{
		CamRotate = 0.0f - asinf(SineRotateF);
	}
	

	Sqaure = Sqaure + Direction.Z * Direction.Z;
	float SqrtXYZ = sqrtf(Sqaure);
	float SinePitch = Direction.Z / SqrtXYZ;
	CamPitch = 0.0f - asinf(SinePitch);
}

void UCamera::SetFov(float InFov)
{
	Fov = InFov;
	UpdateProj();
}

void UCamera::SetAspectRatio(float InAspectRatio)
{
	AspectRatio = InAspectRatio;
	UpdateProj();
}