#include "Lighting.hlsli"
#include "BasePassRS.hlsli"
#include "Common.hlsli"

cbuffer cbPerObject : register(b0)
{
	float4x4 ObjectWorld;

	float3 Fresnel0;
	float Shiness;
	float3 AmbientLight;
};

cbuffer mainPassObject : register(b1)
{
	float4x4 ViewProj;
	float4x4 LightViewProj;
	float4 CamLocation;

	//for global directional lighting
	float4 LightDirection;
	float4 LightStrength;
};

Texture2D gShadowMap : register(t0);

[RootSignature(RenderCore_RootSig)]
float4 main(VertexOut Pin) : SV_Target
{
	//get shadow map info
	uint Width, Height, NumMips;
	gShadowMap.GetDimensions(0, Width, Height, NumMips);
	float4 ShadowUV = Pin.ShadowPosH / Pin.ShadowPosH.w;
	//texel uv size of shadowmap
	int ShadowMapX = round(ShadowUV.x * Width);
	int ShadowMapY = round(ShadowUV.y * Height);
	float DepthInMap = gShadowMap.Load(int3(ShadowMapX, ShadowMapY, 0));
	float ShadowFactor = 1.0f;
	//if (ShadowUV.z > DepthInMap)
	//{
	//	//in shadow
	//	ShadowFactor = 0.5f;
	//}

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
	
	return Pin.Color * ShadowFactor;
}