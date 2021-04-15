#include "Lighting.hlsli"
#include "BasePassRS.hlsli"

cbuffer cbPerObject : register(b0)
{
	float4x4 ObjectWorld;

	float3 Fresnel0;
	float Shiness;
	float3 AmbientLight;
};

cbuffer manPassObject : register(b1)
{
	float4x4 ViewProj;
	float4 CamLocation;

	//for global directional lighting
	float4 LightDirection;
	float4 LightStrength;
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
	Mat.Fresnel0= Fresnel0;
	Mat.Shiness= Shiness;
	Mat.AmbientLight= AmbientLight;
	
	
	DirectionLight Light;
	Light.Strength = float3(LightStrength.x, LightStrength.y, LightStrength.z);
	Light.Strength = normalize(Light.Strength);
	Light.Direction = float3(LightDirection.x, LightDirection.y, LightDirection.z);
	Light.Direction = normalize(Light.Direction);
	
	float3 ViewDirection = float3(CamLocation.x, CamLocation.y, CamLocation.z) - Pin.PosW;
	ViewDirection = normalize(ViewDirection);
	Pin.Normal = normalize(Pin.Normal);
	
	float3 OutColor = ComputeBlinnPhone_DirectionalLight(Light, Mat, Pin.Normal, ViewDirection);
	//return float4(OutColor, 1.0f);
	
	return Pin.Color;
}