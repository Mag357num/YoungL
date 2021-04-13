#pragma once
#include "Utilities.h"

class FCamera
{
public:
	FCamera();

	~FCamera();

	FMatrix* GetCameraView(){return &View;}
	FMatrix* GetCameraProj(){return &Proj;}
	FVector4D* GetCameraLoc(){return &CamPos;}

	void Pitch(float Dy);
	void Rotate(float Dx);

	void SetFov(float InFov);
	void SetAspectRatio(float InAspectRatio);

	void SetCameraLocation(FVector InNewLoc);
	void SetCameraTargetLoc(FVector InNewTarget);

	bool CameraInfoDirty() {
		return IsCamerInfoDirty;
	}

	//called from game tick
	void ResetDirtyFlat(){IsCamerInfoDirty = false;}
private:

	void UpdateView();
	void UpdateProj();
	
	FMatrix View;
	FMatrix Proj;

	FVector4D CamPos;
	FVector CamTarget;
	FVector CamUp;

	float AspectRatio;
	float NearPlan;
	float FarPlan;
	float Fov;

	//used for pitch and rotate
	float Radius;

	// save dirty state
	bool IsCamerInfoDirty;

};

