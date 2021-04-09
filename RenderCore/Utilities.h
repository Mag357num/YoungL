#pragma once
#include <vector>

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
		Content[2][0] = m20; Content[2][1] = m21; Content[2][2] = m02; Content[2][3] = m23;
		Content[3][0] = m30; Content[3][1] = m31; Content[3][2] = m32; Content[3][3] = m33;
	}
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
		:WorldViewProj(1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f)
	{

	}

	FMatrix WorldViewProj;

	FVector CameraLocation = FVector(0.0f, 0.0f, 0.0f);
};

