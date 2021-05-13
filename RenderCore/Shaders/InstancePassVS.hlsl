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

Texture2D InstatnceData : register(t0);

//[RootSignature(RenderCore_RootSig)]
VertexOut main(VertexIn Vin, uint instanceID : SV_InstanceID)
{
	VertexOut vout;

	// Transform to homogeneous clip space.
	float4 PosW = mul(float4(Vin.Pos, 1.0f), ObjectWorld);
	vout.PosH = mul(PosW, ViewProj);
	vout.ShadowPosH = mul(PosW, LightViewProj);
	// Just pass vertex color into the pixel shader.
	vout.Uv = Vin.Uv;
	vout.Normal = normalize(Vin.Normal);
	vout.Color = float4(vout.Normal * 0.5f + 0.5f, 1.0f);
	//vout.Color = float4(0.5f, 0.5f, 0.5f, 1.0f);
	vout.PosW = Vin.Pos;

	return vout;
}