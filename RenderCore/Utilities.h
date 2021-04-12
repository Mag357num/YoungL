#pragma once
#include <vector>
#include <stdarg.h>

#define PI 3.141592654f
#define TWOPI 6.283185307f
#define INVERSEPI 0.318309886f
#define INVERSE2PI 0.159154943f
#define PIDIV2 1.570796327f
#define PIDIV4 0.785398163f


struct FVector2D
{
	FVector2D(float InX, float InY)
		:X(InX),
		Y(InY)
	{

	}
	FVector2D(){}
	float X;
	float Y;
};

struct FVector
{
	FVector(float InX, float InY, float InZ)
		:X(InX),
		Y(InY),
		Z(InZ)
	{

	}
	FVector(){}
	float X;
	float Y;
	float Z;
};

struct FVector4D
{
	FVector4D(float InX, float InY, float InZ, float InW)
		:X(InX),
		Y(InY),
		Z(InZ),
		W(InW)
	{

	}

	FVector4D() {}
	float X;
	float Y;
	float Z;
	float W;
};

struct FMatrix
{
	FMatrix(float m00, float m01, float m02, float m03,
		float m10, float m11, float m12, float m13,
		float m20, float m21, float m22, float m23,
		float m30, float m31, float m32, float m33
	)
	{
		Content[0][0] = m00; Content[0][1] = m01; Content[0][2] = m02; Content[0][3] = m03;
		Content[1][0] = m10; Content[1][1] = m11; Content[1][2] = m12; Content[1][3] = m13;
		Content[2][0] = m20; Content[2][1] = m21; Content[2][2] = m22; Content[2][3] = m23;
		Content[3][0] = m30; Content[3][1] = m31; Content[3][2] = m32; Content[3][3] = m33;
	}

	FMatrix(FVector4D Row0,
		FVector4D Row1,
		FVector4D Row2,
		FVector4D Row3
	)
	{
		Content[0][0] = Row0.X; Content[0][1] = Row0.Y; Content[0][2] = Row0.Z; Content[0][3] = Row0.W;
		Content[1][0] = Row1.X; Content[1][1] = Row1.Y; Content[1][2] = Row1.Z; Content[1][3] = Row1.W;
		Content[2][0] = Row2.X; Content[2][1] = Row2.Y; Content[2][2] = Row2.Z; Content[2][3] = Row2.W;
		Content[3][0] = Row3.X; Content[3][1] = Row3.Y; Content[3][2] = Row3.Z; Content[3][3] = Row3.W;
	}

	FMatrix(){}

	FVector4D GetRow3(){return FVector4D(Content[3][0], Content[3][1], Content[3][2], Content[3][3]);}
	FVector4D GetRow2() { return FVector4D(Content[2][0], Content[2][1], Content[2][2], Content[2][3]); }
	FVector4D GetRow1() { return FVector4D(Content[1][0], Content[1][1], Content[1][2], Content[1][3]); }
	FVector4D GetRow0() { return FVector4D(Content[0][0], Content[0][1], Content[0][2], Content[0][3]); }

	//------------------------------------------------------------------------------
	// Perform a 4x4 matrix multiply by a 4x4 matrix
	static FMatrix MatrixMultiply
	(
		FMatrix M1,
		FMatrix M2
	)
	{
		FMatrix Result;
		// Cache the invariants in registers
		float x = M1.Content[0][0];
		float y = M1.Content[0][1];
		float z = M1.Content[0][2];
		float w = M1.Content[0][3];
		// Perform the operation on the first row
		Result.Content[0][0] = (M2.Content[0][0] * x) + (M2.Content[1][0] * y) + (M2.Content[2][0] * z) + (M2.Content[3][0] * w);
		Result.Content[0][1] = (M2.Content[0][1] * x) + (M2.Content[1][1] * y) + (M2.Content[2][1] * z) + (M2.Content[3][1] * w);
		Result.Content[0][2] = (M2.Content[0][2] * x) + (M2.Content[1][2] * y) + (M2.Content[2][2] * z) + (M2.Content[3][2] * w);
		Result.Content[0][3] = (M2.Content[0][3] * x) + (M2.Content[1][3] * y) + (M2.Content[2][3] * z) + (M2.Content[3][3] * w);
		// Repeat for all the other rows
		x = M1.Content[1][0];
		y = M1.Content[1][1];
		z = M1.Content[1][2];
		w = M1.Content[1][3];
		Result.Content[1][0] = (M2.Content[0][0] * x) + (M2.Content[1][0] * y) + (M2.Content[2][0] * z) + (M2.Content[3][0] * w);
		Result.Content[1][1] = (M2.Content[0][1] * x) + (M2.Content[1][1] * y) + (M2.Content[2][1] * z) + (M2.Content[3][1] * w);
		Result.Content[1][2] = (M2.Content[0][2] * x) + (M2.Content[1][2] * y) + (M2.Content[2][2] * z) + (M2.Content[3][2] * w);
		Result.Content[1][3] = (M2.Content[0][3] * x) + (M2.Content[1][3] * y) + (M2.Content[2][3] * z) + (M2.Content[3][3] * w);
		x = M1.Content[2][0];
		y = M1.Content[2][1];
		z = M1.Content[2][2];
		w = M1.Content[2][3];
		Result.Content[2][0] = (M2.Content[0][0] * x) + (M2.Content[1][0] * y) + (M2.Content[2][0] * z) + (M2.Content[3][0] * w);
		Result.Content[2][1] = (M2.Content[0][1] * x) + (M2.Content[1][1] * y) + (M2.Content[2][1] * z) + (M2.Content[3][1] * w);
		Result.Content[2][2] = (M2.Content[0][2] * x) + (M2.Content[1][2] * y) + (M2.Content[2][2] * z) + (M2.Content[3][2] * w);
		Result.Content[2][3] = (M2.Content[0][3] * x) + (M2.Content[1][3] * y) + (M2.Content[2][3] * z) + (M2.Content[3][3] * w);
		x = M1.Content[3][0];
		y = M1.Content[3][1];
		z = M1.Content[3][2];
		w = M1.Content[3][3];
		Result.Content[3][0] = (M2.Content[0][0] * x) + (M2.Content[1][0] * y) + (M2.Content[2][0] * z) + (M2.Content[3][0] * w);
		Result.Content[3][1] = (M2.Content[0][1] * x) + (M2.Content[1][1] * y) + (M2.Content[2][1] * z) + (M2.Content[3][1] * w);
		Result.Content[3][2] = (M2.Content[0][2] * x) + (M2.Content[1][2] * y) + (M2.Content[2][2] * z) + (M2.Content[3][2] * w);
		Result.Content[3][3] = (M2.Content[0][3] * x) + (M2.Content[1][3] * y) + (M2.Content[2][3] * z) + (M2.Content[3][3] * w);
		return Result;
	}

	FMatrix operator* (FMatrix InMat)const
	{
		FMatrix M = MatrixMultiply(*this, InMat);	
		return M;
	}


	//matrix content
	float Content[4][4];

};



struct FVertex
{
	FVertex(){}
	FVector Position;
	FVector Normal;
	FVector2D Uv;
};

class FGeometry
{
public:
	FGeometry(std::vector<FVertex>& InVertices, std::vector<uint32_t> InIndices)
	:Vertices(InVertices),
	Indices(InIndices)
	{

	}

	FGeometry(const FGeometry& rhs) = delete;
	FGeometry& operator=(const FGeometry& rhs) = delete;

	~FGeometry()
	{
		Vertices.empty();
		Indices.empty();
	}

	std::vector<FVertex>& GetVertices(){return Vertices;}
	std::vector<uint32_t>& GetIndices(){return Indices;}

public:

private:
	std::vector<FVertex> Vertices;
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
	FVector Fresnel0 = FVector(0.04f, 0.04f, 0.04f);
	float Shiness = 0.7f;
	FVector AmbientLight = FVector(0.1f, 0.1f, 0.1f);
};

struct FSceneConstant
{
	FSceneConstant(){}

	FMatrix ViewProj;
	FVector4D CamLocation;

	//saved for global directional lighting
	FVector4D LightDirection;
	FVector4D LightStrength;
}; 


namespace Utilities
{
//	inline void Print(const char* msg) { printf("%s", msg); }
//	inline void Print(const wchar_t* msg) { wprintf(L"%ws", msg); }
//
//	inline void Printf(const char* format, ...)
//	{
//		char buffer[256];
//		va_list ap;
//		va_start(ap, format);
//		vsprintf_s(buffer, 256, format, ap);
//		Print(buffer);
//	}
//
//	inline void Printf(const wchar_t* format, ...)
//	{
//		wchar_t buffer[256];
//		va_list ap;
//		va_start(ap, format);
//		vswprintf(buffer, 256, format, ap);
//		Print(buffer);
//	}
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

	static FMatrix IdentityMatrix = FMatrix(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);


	static bool ScalarNearEqual
	(
		float S1,
		float S2,
		float Epsilon
	)
	{
		float Delta = S1 - S2;
		return (fabsf(Delta) <= Epsilon);
	}
//
	static void ScalarSinCos
	(
		float* pSin,
		float* pCos,
		float  Value
	)
	{

		// Map Value to y in [-pi,pi], x = 2*pi*quotient + remainder.
		float quotient = Value * INVERSE2PI;
		if (Value >= 0.0f)
		{
			quotient = static_cast<float>(static_cast<int>(quotient + 0.5f));
		}
		else
		{
			quotient = static_cast<float>(static_cast<int>(quotient - 0.5f));
		}
		float y = Value - quotient * TWOPI;

		// Map y to [-pi/2,pi/2] with sin(y) = sin(Value).
		float sign;
		if (y > PIDIV2)
		{
			y = PI - y;
			sign = -1.0f;
		}
		else if (y < -PIDIV2)
		{
			y = -PI - y;
			sign = -1.0f;
		}
		else
		{
			sign = +1.0f;
		}

		float y2 = y * y;

		// 11-degree minimax approximation
		*pSin = (((((-2.3889859e-08f * y2 + 2.7525562e-06f) * y2 - 0.00019840874f) * y2 + 0.0083333310f) * y2 - 0.16666667f) * y2 + 1.0f) * y;

		// 10-degree minimax approximation
		float p = ((((-2.6051615e-07f * y2 + 2.4760495e-05f) * y2 - 0.0013888378f) * y2 + 0.041666638f) * y2 - 0.5f) * y2 + 1.0f;
		*pCos = sign * p;
	}
//
	static FMatrix MatrixPerspectiveFovLH
	(
		float FovAngleY,
		float AspectRatio,
		float NearZ,
		float FarZ
	)
	{
		float    SinFov;
		float    CosFov;
		ScalarSinCos(&SinFov, &CosFov, 0.5f * FovAngleY);

		float Height = CosFov / SinFov;
		float Width = Height / AspectRatio;
		float fRange = FarZ / (FarZ - NearZ);

		FMatrix M;
		M.Content[0][0] = Width;
		M.Content[0][1] = 0.0f;
		M.Content[0][2] = 0.0f;
		M.Content[0][3] = 0.0f;

		M.Content[1][0] = 0.0f;
		M.Content[1][1] = Height;
		M.Content[1][2] = 0.0f;
		M.Content[1][3] = 0.0f;

		M.Content[2][0] = 0.0f;
		M.Content[2][1] = 0.0f;
		M.Content[2][2] = fRange;
		M.Content[2][3] = 1.0f;

		M.Content[3][0] = 0.0f;
		M.Content[3][1] = 0.0f;
		M.Content[3][2] = -fRange * NearZ;
		M.Content[3][3] = 0.0f;
		return M;
	}


	static FVector4D VectorSubtract
	(
		FVector4D V1,
		FVector4D V2
	)
	{
		FVector4D Result = FVector4D(V1.X - V2.X,
			V1.Y - V2.Y,
			V1.Z - V2.Z,
			V1.W - V2.W);
		return Result;
	}



	static FVector4D Vector3Dot
	(
		FVector4D V1,
		FVector4D V2
	)
	{
		float fValue = V1.X * V2.X + V1.Y * V2.Y + V1.Z * V2.Z;
		FVector4D vResult;
		vResult.X =
			vResult.Y =
			vResult.Z =
			vResult.W = fValue;
		return vResult;
	}

	//------------------------------------------------------------------------------

	static FVector4D Vector3Cross
	(
		FVector4D V1,
		FVector4D V2
	)
	{
		// [ V1.y*V2.z - V1.z*V2.y, V1.z*V2.x - V1.x*V2.z, V1.x*V2.y - V1.y*V2.x ]
		FVector4D vResult = FVector4D(
			(V1.Y * V2.Z) - (V1.Z * V2.Y),
			(V1.Z * V2.X) - (V1.X * V2.Z),
			(V1.X * V2.Y) - (V1.Y * V2.X),
			0.0f
		);
		return vResult;
	}


	static FVector4D VectorSqrt
	(
		FVector4D V
	)
	{
		FVector4D Result = FVector4D(
			sqrtf(V.X),
			sqrtf(V.Y),
			sqrtf(V.Z),
			sqrtf(V.W)
		);
		return Result;
	}

	static FVector4D Vector3Length
	(
		FVector4D V
	)
	{

		FVector4D Result;

		Result = Vector3Dot(V, V);
		Result = VectorSqrt(Result);

		return Result;
	}

	static FVector4D Vector3Normalize
	(
		FVector4D V
	)
	{
		float fLength;
		FVector4D vResult;

		vResult = Vector3Length(V);
		fLength = vResult.X;

		// Prevent divide by zero
		if (fLength > 0) {
			fLength = 1.0f / fLength;
		}

		vResult.X = V.X * fLength;
		vResult.Y = V.Y * fLength;
		vResult.Z = V.Z * fLength;
		vResult.W = V.W * fLength;
		return vResult;
	}

	static FVector4D VectorMergeXY
	(
		FVector4D V1,
		FVector4D V2
	)
	{
		FVector4D Result = FVector4D(
			V1.X,
			V2.X,
			V1.Y,
			V2.Y
		);
		return Result;

	}

	//------------------------------------------------------------------------------

	static FVector4D VectorMergeZW
	(
		FVector4D V1,
		FVector4D V2
	)
	{

		FVector4D Result = FVector4D(
			V1.Z,
			V2.Z,
			V1.W,
			V2.W
		);
		return Result;
	}

	static FMatrix MatrixTranspose
	(
		FMatrix M
	)
	{

		// Original matrix:
		//
		//     m00m01m02m03
		//     m10m11m12m13
		//     m20m21m22m23
		//     m30m31m32m33

		 
		FVector4D Row0_P = Utilities::VectorMergeXY(M.GetRow0(), M.GetRow2()); // m00m20m01m21
		FVector4D Row1_P = Utilities::VectorMergeXY(M.GetRow1(), M.GetRow3());// m10m30m11m31
		FVector4D Row2_P = Utilities::VectorMergeZW(M.GetRow0(), M.GetRow2());// m02m22m03m23
		FVector4D Row3_P = Utilities::VectorMergeZW(M.GetRow1(), M.GetRow3());// m12m32m13m33
		FMatrix P(Row0_P, Row1_P, Row2_P, Row3_P);


		FVector4D Row0_MT = Utilities::VectorMergeXY(P.GetRow0(), P.GetRow1());// m00m10m20m30
		FVector4D Row1_MT = Utilities::VectorMergeZW(P.GetRow0(), P.GetRow1());// m01m11m21m31
		FVector4D Row2_MT = Utilities::VectorMergeXY(P.GetRow2(), P.GetRow3());// m02m12m22m32
		FVector4D Row3_MT = Utilities::VectorMergeZW(P.GetRow2(), P.GetRow3());// m03m13m23m33
		FMatrix MT(Row0_MT, Row1_MT, Row2_MT, Row3_MT);
		return MT;
	}


	static FMatrix MatrixLookToLH
	(
		FVector4D EyePosition,
		FVector4D EyeDirection,
		FVector4D UpDirection
	)
	{

		FVector4D R2 = Vector3Normalize(EyeDirection);

		FVector4D R0 = Vector3Cross(UpDirection, R2);
		R0 = Vector3Normalize(R0);

		FVector4D R1 = Vector3Cross(R2, R0);

		FVector4D NegEyePosition = FVector4D(-EyePosition.X, -EyePosition.Y, -EyePosition.Z, -EyePosition.W);

		FVector4D D0 = Vector3Dot(R0, NegEyePosition);
		FVector4D D1 = Vector3Dot(R1, NegEyePosition);
		FVector4D D2 = Vector3Dot(R2, NegEyePosition);

		FVector4D Row0_M = FVector4D(R0.X, R0.Y, R0.Z, D0.W);
		//M.r[0] = VectorSelect(D0, R0, g_XMSelect1110.v);
		FVector4D Row1_M = FVector4D(R1.X, R1.Y, R1.Z, D1.W);
		//M.r[1] = XMVectorSelect(D1, R1, g_XMSelect1110.v);
		FVector4D Row2_M = FVector4D(R2.X, R2.Y, R2.Z, D2.W);
		//M.r[2] = XMVectorSelect(D2, R2, g_XMSelect1110.v);
		FVector4D Row3_M = FVector4D(0.0f, 0.0f, 0.0f, 1.0f);
		//M.r[3] = g_XMIdentityR3.v;
		FMatrix M(Row0_M, Row1_M, Row2_M, Row3_M);

		M = MatrixTranspose(M);

		return M;
	}


	static FMatrix MatrixLookAtLH
	(
		FVector4D EyePosition,
		FVector4D FocusPosition,
		FVector4D UpDirection
	)
	{
		FVector4D EyeDirection = VectorSubtract(FocusPosition, EyePosition);
		return MatrixLookToLH(EyePosition, EyeDirection, UpDirection);
	}
};