#pragma once
#include "Object.h"
#include "../Utilities.h"
class UTexture : public UObject
{
public:
	UTexture(std::string TypeName, UINT InWidth, UINT InHeight);

	~UTexture();

	virtual void Serialize()override {}

	virtual void PostLoad()override {}

	virtual void Destroy()override {}

	void InitializeTextureContent(std::vector<FColor>& InColors);
	void InitializeTextureContent(FColor ClearColor);

	static std::shared_ptr<UTexture> CreateEmptyTexture(UINT InWidth, UINT InHeight);
	static std::shared_ptr<UTexture> CreateTextureWithClear(UINT InWidth, UINT InHeight, FColor ClearColor);


	void ModifyColor(UINT Row, UINT Column, FColor InColor);

	void RequestUpdateRenderResource(std::vector<FActorInstanceInfo>& InstanceInfos);

	UINT GetWidth(){return Width;}
	UINT GetHeight() { return Height; }
	std::vector<FColor>* GetAllColors(){return &Colors;}
private:
	
	UINT Width;
	UINT Height;

	std::vector<FColor> Colors;


};

