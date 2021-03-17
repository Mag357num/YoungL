#include "Lighting.hlsl"

cbuffer cbPerObject : register(b0)
{
	float4x4 gWorldViewProj;
	float3 CameraLocation;
};

struct VertexIn
{
	float3 Pos : POSITION;
	float3 Normal : NORMAL;
	float2 Uv : TEXCOORD;
	float4 Color : COLOR;
};

struct VertexOut
{
	float4 PosH : SV_POSITION;
	float4 Color : COLOR;
	float3 Normal : NORMAL;
	float2 Uv : TEXCOORD;
	float3 PosW : POSITION;
};

VertexOut VS(VertexIn Vin)
{
	VertexOut vout;
	
	// Transform to homogeneous clip space.
	vout.PosH = mul(float4(Vin.Pos, 1.0f), gWorldViewProj);
	
	// Just pass vertex color into the pixel shader.
    vout.Color = Vin.Color;
	
	vout.Uv = Vin.Uv;
	vout.Normal = normalize(Vin.Normal);
    vout.PosW=Vin.Pos;
	
    return vout;
}

float4 PS(VertexOut Pin) : SV_Target
{
	//construct material and light
	Material Mat;
	Mat.DiffuseAlbedo=float4(0.1f, 0.7f, 0.7f, 1.0f);
	Mat.Fresnel0=float3(0.04f,0.04f,0.04f);
	Mat.Shiness=0.1f;
	Mat.AmbientLight=float3(0.1f,0.1f,0.1f);
	
	
	DirectionLight Light;
	Light.Position=float3(0.0f, 0.0f, 0.0f);
	Light.Strength = float3(0.5f, 0.5f, 0.5f);
	Light.Direction = float3(0.0f ,0.0f, -1.0f);
	
	float3 ViewDirection = CameraLocation - Pin.PosW;
	ViewDirection = normalize(ViewDirection);

	float3 OutColor = ComputeBlinnPhone_DirectionalLight(Light, Mat, Pin.Normal, ViewDirection);
	return float4(OutColor, 1.0f);
	
	//return Pin.Color;
}