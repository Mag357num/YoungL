#include "Lighting.hlsli"
#include "RenderCoreRS.hlsli"

cbuffer cbPerObject : register(b0)
{
	float4x4 gWorldViewProj;
	float3 CameraLocation;
};

struct VertexOut
{
	float4 PosH : SV_POSITION;
	float4 Color : COLOR;
	float3 Normal : NORMAL;
	float2 Uv : TEXCOORD;
	float3 PosW : POSITION;
};

[RootSignature(RenderCore_RootSig)]
float4 main(VertexOut Pin) : SV_Target
{
	//construct material and light
	Material Mat;
	//Mat.DiffuseAlbedo=float4(0.1f, 0.7f, 0.7f, 1.0f);
	Mat.DiffuseAlbedo=Pin.Color;
	Mat.Fresnel0=float3(0.04f,0.04f,0.04f);
	Mat.Shiness=0.7f;
	Mat.AmbientLight=float3(0.1f,0.1f,0.1f);
	
	
	DirectionLight Light;
	Light.Position=float3(0.0f, 0.0f, 0.0f);
	Light.Strength = float3(0.5f, 0.5f, 0.5f);
	Light.Strength = normalize(Light.Strength);
	Light.Direction = float3(-1.0f ,-1.0f, -1.0f);
	Light.Direction = normalize(Light.Direction);
	
	float3 ViewDirection = CameraLocation - Pin.PosW;
	ViewDirection = normalize(ViewDirection);
	Pin.Normal = normalize(Pin.Normal);
	
	float3 OutColor = ComputeBlinnPhone_DirectionalLight(Light, Mat, Pin.Normal, ViewDirection);
	//return float4(OutColor, 1.0f);
	
	return Pin.Color;
}