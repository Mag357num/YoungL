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
	Colors.clear();
}

void UTexture::InitializeTextureContent(std::vector<FColor>& InColors)
{
	Colors.clear();

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
	Colors.clear();

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

	Colors[Index].ColorF[0] = InColor.ColorF[0];
	Colors[Index].ColorF[1] = InColor.ColorF[1];
	Colors[Index].ColorF[2] = InColor.ColorF[2];
	Colors[Index].ColorF[3] = InColor.ColorF[3];
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
void UTexture::RequestUpdateRenderResource(std::vector<FActorInstanceInfo>& InstanceInfos)
{
	size_t InstanceSize = InstanceInfos.size();
	if (InstanceSize > Width * Height * 0.25f)
	{
		//TODO:out of size warning
		Utilities::Print("Instance size is out of texture size");

		return;
	}

	for (size_t Index = 0; Index < InstanceInfos.size(); Index++)
	{
		FMatrix Transform = FMath::MatrixAffineTransformation(InstanceInfos[Index].Scaling, InstanceInfos[Index].Rotation, InstanceInfos[Index].Translation);
		int PixelWidth = (int)round(0.5f * Width);
		//int PixelHeight = round(0.5f * Height);
		
		int Row = (int)Index / PixelWidth;
		int Col = Index % PixelWidth;

		// split matrix to 4 pixel
		// |Row0|Row1|
		// |Row2|Row3|
		//

		int PixelIndex0 = (Row * 2) * Width  + Col * 2;
		Colors[PixelIndex0].ColorF[0] = Transform.GetRow0().X;
		Colors[PixelIndex0].ColorF[1] = Transform.GetRow0().Y;
		Colors[PixelIndex0].ColorF[2] = Transform.GetRow0().Z;
		Colors[PixelIndex0].ColorF[3] = Transform.GetRow0().W;

		int PixelIndex1 = (Row * 2) * Width + Col * 2 +1;
		Colors[PixelIndex1].ColorF[0] = Transform.GetRow1().X;
		Colors[PixelIndex1].ColorF[1] = Transform.GetRow1().Y;
		Colors[PixelIndex1].ColorF[2] = Transform.GetRow1().Z;
		Colors[PixelIndex1].ColorF[3] = Transform.GetRow1().W;

		int PixelIndex2 = (Row * 2 + 1) * Width + Col * 2;
		Colors[PixelIndex2].ColorF[0] = Transform.GetRow2().X;
		Colors[PixelIndex2].ColorF[1] = Transform.GetRow2().Y;
		Colors[PixelIndex2].ColorF[2] = Transform.GetRow2().Z;
		Colors[PixelIndex2].ColorF[3] = Transform.GetRow2().W;

		int PixelIndex3 = (Row * 2 + 1) * Width + Col * 2 +1;
		Colors[PixelIndex3].ColorF[0] = Transform.GetRow3().X;
		Colors[PixelIndex3].ColorF[1] = Transform.GetRow3().Y;
		Colors[PixelIndex3].ColorF[2] = Transform.GetRow3().Z;
		Colors[PixelIndex3].ColorF[3] = Transform.GetRow3().W;
	}
}