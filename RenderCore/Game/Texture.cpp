#include "pch.h"
#include "Texture.h"

UTexture::UTexture(std::string TypeName, UINT InWidth, UINT InHeight)
	:UObject(TypeName),
	Width(InWidth),
	Height(InHeight)
{

}

UTexture::~UTexture()
{
	if (!Colors.empty())
	{
	}
}

void UTexture::InitializeTextureContent(std::vector<FColor>& InColors)
{
	if (!Colors.empty())
	{
	}

	for (UINT Row = 0; Row < Height; ++Row)
	{
		for (UINT Column = 0; Column < Width; ++Column)
		{
			UINT Index = Row * Height + Column;
			//Colors.push_back(FColorPreset::LightBlue); 
			Colors.push_back(InColors[Index]);
		}
	}
}

void UTexture::InitializeTextureContent(FColor ClearColor)
{
	if (!Colors.empty())
	{
	}

	for (UINT Row = 0; Row < Height; ++Row)
	{
		for (UINT Column = 0; Column < Width; ++Column)
		{
			//Colors.push_back(FColorPreset::LightBlue);
			Colors.push_back(ClearColor);
		}
	}
}

void UTexture::ModifyColor(UINT Row, UINT Column, FColor InColor)
{
	UINT Index = Row * Height + Column;

	Colors[Index].Type = InColor.Type;
	if (InColor.Type == ColorType_SRGB)
	{
		Colors[Index].ColorI[0] = InColor.ColorI[0];
		Colors[Index].ColorI[1] = InColor.ColorI[1];
		Colors[Index].ColorI[2] = InColor.ColorI[2];
		Colors[Index].ColorI[3] = InColor.ColorI[3];
	}
	else
	{
		Colors[Index].ColorF[0] = InColor.ColorF[0];
		Colors[Index].ColorF[1] = InColor.ColorF[1];
		Colors[Index].ColorF[2] = InColor.ColorF[2];
		Colors[Index].ColorF[3] = InColor.ColorF[3];
	}
}

std::shared_ptr<UTexture> UTexture::CreateEmptyTexture(UINT InWidth, UINT InHeight)
{
	std::shared_ptr<UTexture> Ret = std::make_shared<UTexture>("Texture", InWidth, InHeight);
	return Ret;
}

std::shared_ptr<UTexture> UTexture::CreateTextureWithClear(UINT InWidth, UINT InHeight, FColor ClearColor)
{
	std::shared_ptr<UTexture> Ret = std::make_shared<UTexture>("Texture", InWidth, InHeight);
	Ret->InitializeTextureContent(ClearColor);
	return Ret;
}

//TODO: send request to render thread 
//to update binded render resource
void UTexture::RequestUpdateRenderResource()
{

}