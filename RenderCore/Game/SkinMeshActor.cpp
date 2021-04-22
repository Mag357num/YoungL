#include "pch.h"
#include "SkinMeshActor.h"

ASkinMeshActor::ASkinMeshActor()
{
	SkinedData = new FSkinedData();
}

ASkinMeshActor::~ASkinMeshActor()
{
	if (SkinedData)
	{
		delete SkinedData;
		SkinedData = nullptr;
	}
}

void ASkinMeshActor::Tick(float DeltaTime)
{

}