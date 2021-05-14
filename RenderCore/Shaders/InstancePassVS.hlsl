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

	int InstanceRow = instanceID / InstanceWidth;
	int InstanceCol = instanceID % InstanceWidth;

	int2 PixelRow0 = int2(InstanceRow * 2, InstanceCol * 2);
	int2 PixelRow1 = int2(InstanceRow * 2, InstanceCol * 2 + 1);
	int2 PixelRow2 = int2(InstanceRow * 2 + 1, InstanceCol * 2);
	int2 PixelRow3 = int2(InstanceRow * 2 + 1, InstanceCol * 2 + 1);

	float4 Row0Data = InstatnceData.Load(int3(PixelRow0, 0));
	float4 Row1Data = InstatnceData.Load(int3(PixelRow1, 0));
	float4 Row2Data = InstatnceData.Load(int3(PixelRow2, 0));
	float4 Row3Data = InstatnceData.Load(int3(PixelRow3, 0));

	// Transform to homogeneous clip space.
	float4 PosW = mul(float4(Vin.Pos, 1.0f), ObjectWorld);
	vout.PosH = mul(PosW, ViewProj);
	vout.ShadowPosH = mul(PosW, LightViewProj);
	// Just pass vertex color into the pixel shader.
	vout.Uv = Vin.Uv;
	
	vout.Normal = normalize(Vin.Normal);

	//vout.Color = float4(vout.Normal * 0.5f + 0.5f, 1.0f);
	//vout.Color = float4(0.5f, 0.5f, 0.5f, 1.0f);
	vout.Color = Row0Data * 0.25f + Row1Data * 0.25f + Row2Data * 0.25f + Row3Data * 0.25f;

	vout.PosW = Vin.Pos;

	return vout;
}