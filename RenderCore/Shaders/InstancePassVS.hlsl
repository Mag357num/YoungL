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

int2 RenderTargetSize : register(b2);

Texture2D InstatnceData : register(t1);

//[RootSignature(RenderCore_RootSig)]
VertexOut main(VertexIn Vin, uint instanceID : SV_InstanceID)
{
	VertexOut vout;

	//instance id to pixel
	int InstanceWidth = round(RenderTargetSize[0] * 0.5f);
	int InstanceHeight = round(RenderTargetSize[1] * 0.5f);

	int InstanceRow = floor(1.0f * instanceID / InstanceWidth);
	int InstanceCol = instanceID - InstanceRow * InstanceWidth;

	int2 PixelRow0 = int2(InstanceCol * 2,		InstanceRow * 2);
	int2 PixelRow1 = int2(InstanceCol * 2 + 1,	InstanceRow * 2);
	int2 PixelRow2 = int2(InstanceCol * 2,		InstanceRow * 2 + 1);
	int2 PixelRow3 = int2(InstanceCol * 2 + 1,	InstanceRow * 2 + 1);

	//float4 Row0Data = InstatnceData.Load(int3(0, 1, 0));
	float4 Row0Data = InstatnceData.Load(int3(PixelRow0, 0));
	float4 Row1Data = InstatnceData.Load(int3(PixelRow1, 0));
	float4 Row2Data = InstatnceData.Load(int3(PixelRow2, 0));
	float4 Row3Data = InstatnceData.Load(int3(PixelRow3, 0));

	float4x4 InstanceSpace = {
				Row0Data,
				Row1Data,
				Row2Data,
				Row3Data
	};

	float4 InstanceSpaceLoc = mul(float4(Vin.Pos, 1.0f), InstanceSpace);

	// Transform to homogeneous clip space.
	float4 PosW = mul(InstanceSpaceLoc, ObjectWorld);
	vout.PosH = mul(PosW, ViewProj);
	vout.ShadowPosH = mul(PosW, LightViewProj);

	// Just pass vertex color into the pixel shader.
	vout.Uv = Vin.Uv;
	vout.Normal = normalize(Vin.Normal);
	vout.Color = float4(vout.Normal * 0.5f + 0.5f, 1.0f);
	vout.PosW = Vin.Pos;

	return vout;
}