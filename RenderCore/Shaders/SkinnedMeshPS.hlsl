#include "Lighting.hlsli"
#include "SkinnedMeshRS.hlsli"
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

[RootSignature(SkinnedMesh_RootSig)]
float4 main(VertexOut Pin) : SV_Target
{
	//get shadow map info
	uint Width, Height, NumMips;
	gShadowMap.GetDimensions(0, Width, Height, NumMips);
	float4 ShadowUV = Pin.ShadowPosH / Pin.ShadowPosH.w;
	//texel uv size of shadowmap
	uint ShadowMapX = floor(ShadowUV.x * Width);
	uint ShadowMapY = floor(ShadowUV.y * Height);

	uint2 PixelPos[9] =
	{
		uint2(ShadowMapX - 1, ShadowMapY - 1), uint2(ShadowMapX, ShadowMapY - 1), uint2(ShadowMapX + 1, ShadowMapY - 1),
		uint2(ShadowMapX - 1, ShadowMapY), uint2(ShadowMapX, ShadowMapY), uint2(ShadowMapX + 1, ShadowMapY),
		uint2(ShadowMapX - 1, ShadowMapY + 1), uint2(ShadowMapX, ShadowMapY + 1), uint2(ShadowMapX + 1, ShadowMapY + 1)
	};

	//creat percentage closer filter
	float ShadowFactor = 1.0f;
	if (ShadowMapX > 0 && ShadowMapX < Width - 1 && ShadowMapY > 0 && ShadowMapY < Height - 1)
	{
		[unroll]
		for (int LocIndex = 0; LocIndex < 9; ++LocIndex)
		{
			float DepthInMap = gShadowMap.Load(int3(PixelPos[LocIndex], 0)).r;
			if (ShadowUV.z > DepthInMap + 0.0003f)
			{
				//in shadow
				ShadowFactor += 1.0f;
			}
		}

		//get percentage 
		ShadowFactor *= 0.11111f;

		ShadowFactor = 1.0f - ShadowFactor;
	}

	//construct material and light
	Material Mat;
	//Mat.DiffuseAlbedo=float4(0.4f, 0.4f, 0.4f, 1.0f);
	Mat.DiffuseAlbedo = Pin.Color;
	Mat.Fresnel0 = Fresnel0;
	Mat.Shiness = Shiness;
	Mat.AmbientLight = AmbientLight;


	DirectionLight Light;
	Light.Strength = float3(LightStrength.x, LightStrength.y, LightStrength.z);
	Light.Strength = normalize(Light.Strength);
	Light.Direction = float3(LightDirection.x, LightDirection.y, LightDirection.z);
	Light.Direction = normalize(Light.Direction);

	float3 ViewDirection = float3(CamLocation.x, CamLocation.y, CamLocation.z) - Pin.PosW;
	ViewDirection = normalize(ViewDirection);
	Pin.Normal = normalize(Pin.Normal);

	float3 OutColor = ComputeBlinnPhone_DirectionalLight(Light, Mat, Pin.Normal, ViewDirection, ShadowFactor);
	return float4(OutColor, 1.0f);

}