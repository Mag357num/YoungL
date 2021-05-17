#pragma once
#include <vector>
#include <stdarg.h>
#include <stdlib.h>
#include <iostream>
#include "Math/Math.h"

using namespace std;

template<typename T>
class FGeometry
{
public:
	FGeometry(std::vector<T>& InVertices, std::vector<uint32_t> InIndices)
	:Vertices(InVertices),
	Indices(InIndices)
	{

	}

	FGeometry(const FGeometry& rhs) = delete;
	FGeometry& operator=(const FGeometry& rhs) = delete;

	~FGeometry()
	{
		if (!Vertices.empty())
		{
			printf("Empty Error!");
		}
		
		if (!Indices.empty())
		{
			printf("Empty Error!");
		}
	}

	std::vector<T>& GetVertices(){return Vertices;}
	std::vector<uint32_t>& GetIndices(){return Indices;}

public:

private:
	std::vector<T> Vertices;
	std::vector<uint32_t> Indices;

};

struct FObjectConstants
{
	FObjectConstants()
		:ObjectWorld(1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f)
	{
	}

	FMatrix ObjectWorld;


	//saved for material
	FVector Fresnel0 = FVector(0.9f, 0.9f, 0.9f);
	float Shiness = 0.1f;
	FVector AmbientLight = FVector(0.2f, 0.2f, 0.2f);
};

struct FBoneTransforms
{
	FBoneTransforms(){}
	FMatrix BoneTransforms[96];
};

struct FMaterial
{
	std::string Name;

	FVector FresnelR0 = {0.1f, 0.1f, 0.1f};
	FVector4D DiffuseAlbedo = {1.0f, 1.0f, 1.0f, 1.0f};
	float Roughness = 0.8f;
	bool AlphaClip = false;

	std::string MaterialTypeName;
	std::string DiffuseMapName;
	std::string NormalMapName;
};

struct FSceneConstant
{
	FSceneConstant(){}

	FMatrix ViewProj;
	FMatrix LightViewProj;
	FVector4D CamLocation;

	//saved for global directional lighting
	FVector4D LightDirection;
	FVector4D LightStrength;
}; 

struct FBoundSphere
{
	FBoundSphere(){}

	FVector4D Center;
	float Radius;
};

enum EColorType
{
	ColorType_SRGB = 0,
	ColorType_Linear
};

struct FColor
{
	FColor(){}

	FColor(int R, int G, int B, int A)
	{

		ColorF[0] = floor(R* 1.0f / 256.0f);
		ColorF[1] = floor(G * 1.0f / 256.0f);
		ColorF[2] = floor(B * 1.0f / 256.0f);
		ColorF[3] = floor(A * 1.0f / 256.0f);
	}

	FColor(float R, float G, float B, float A)
	{

		ColorF[0] = R;
		ColorF[1] = G;
		ColorF[2] = B;
		ColorF[3] = A;
	}

	float ColorF[4];

	static FColor MakeColorFromSRGB(int R, int G, int B, int A )
	{
		FColor Ret;


		Ret.ColorF[0] = floor(R * 1.0f / 256.0f);
		Ret.ColorF[1] = floor(G * 1.0f / 256.0f);
		Ret.ColorF[2] = floor(B * 1.0f / 256.0f);
		Ret.ColorF[3] = floor(A * 1.0f / 256.0f);

		return Ret;
	}

	static FColor MakeColorFromLinear(float R, float G, float B, float A)
	{
		FColor Ret;


		Ret.ColorF[0] = R;
		Ret.ColorF[1] = G;
		Ret.ColorF[2] = B;
		Ret.ColorF[3] = A;

		return Ret;
	}

	////ClearValue light blue
	//ClearValue.Color[0] = 0.678431392f;
	//ClearValue.Color[1] = 0.847058892f;
	//ClearValue.Color[2] = 0.901960850f;
	//ClearValue.Color[3] = 1.0f;//0.678431392f, 0.847058892f, 0.901960850f
};

namespace FColorPreset
{
	static FColor LightBlue = FColor(0.678431392f, 0.847058892f, 0.901960850f, 1.0f);
}

struct FActorInstanceInfo
{
	FVector4D Translation;
	FVector4D Rotation;
	FVector4D Scaling;
};

struct FInstanceData
{
	FVector4D TranslationRow0;
	FVector4D TranslationRow1;
	FVector4D TranslationRow2;
	FVector4D TranslationRow3;
};

namespace Utilities
{
	static void Print(const char* msg) { 
		printf("%s", msg);
		fflush(stdout);
	}
	static void Print(const wchar_t* msg) {
		wprintf(L"%ws", msg);
		fflush(stdout);

	}
//
	static void Printf(const char* format, ...)
	{
		char buffer[256];
		va_list ap;
		va_start(ap, format);
		vsprintf_s(buffer, 256, format, ap);
		Print(buffer);
	}
//
	static void Printf(const wchar_t* format, ...)
	{
		wchar_t buffer[256];
		va_list ap;
		va_start(ap, format);
		vswprintf(buffer, 256, format, ap);
		Print(buffer);
	}
//
//#ifndef RELEASE
//	inline void PrintSubMessage(const char* format, ...)
//	{
//		Print("--> ");
//		char buffer[256];
//		va_list ap;
//		va_start(ap, format);
//		vsprintf_s(buffer, 256, format, ap);
//		Print(buffer);
//		Print("\n");
//	}
//
//	inline void PrintSubMessage(const wchar_t* format, ...)
//	{
//		Print("--> ");
//		wchar_t buffer[256];
//		va_list ap;
//		va_start(ap, format);
//		vswprintf(buffer, 256, format, ap);
//		Print(buffer);
//		Print("\n");
//	}
//
//	inline void PrintSubMessage(void)
//	{
//
//	}
//#endif // RELEASE
//
//	inline size_t HashRange(const uint32_t* const Begin, const uint32_t* const End, size_t Hash)
//	{
//#if ENABLE_SSE_CRC32
//		const  uint64_t* Iter64 = (const uint64_t*)Math::AlignUp(Begin, 8);
//		const uint64_t* const End64 = (const uint64_t*)Math::AlignDown(End, 8);
//
//		// if not 64 aligned start with a single u32
//		if ((uint32_t*)Iter64 > Begin)
//		{
//			Hash = _mm_crc32_u32((uint32_t)Hash, *Begin);
//		}
//
//		//iteratro over consecutive u64 values;
//		while (Iter64 < End64)
//		{
//			Hash = _mm_crc32_u64((uint64_t)Hash, *Iter64++);
//		}
//
//		//if there is a 32-bit remander, accumulate 
//		if ((uint32_t*)Iter64 < End)
//		{
//			Hash = _mm_crc32_u32((uint32_t)Hash, *(uint32_t*)Iter64);
//		}
//
//#else
//		for (const uint32_t* Iter = Begin; Iter < End; ++Iter)
//		{
//			Hash = 16777619U * Hash ^ *Iter;
//		}
//#endif
//		return Hash;
//	}
//
//	template<typename T> inline size_t HashState(const T* StateDesc, size_t Count = 1, size_t Hash = 2166136261U)
//	{
//		static_assert((sizeof(T) & 3) == 0 && alignof(T) >= 4, "state Object is not word-aligned");
//		return HashRange((uint32_t*)StateDesc, (uint32_t*)(StateDesc + Count), Hash);
//	}

}