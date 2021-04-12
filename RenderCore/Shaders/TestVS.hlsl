#include "RenderCoreRS.hlsli"

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

struct VertexIn
{
	float3 Pos : POSITION;
	float3 Normal : NORMAL;
	float2 Uv : TEXCOORD;
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
VertexOut main(VertexIn Vin)
{
	VertexOut vout;
	
	// Transform to homogeneous clip space.
	float4 PosW = mul(float4(Vin.Pos, 1.0f), ObjectWorld);
	vout.PosH = mul(PosW, ViewProj);
	
	// Just pass vertex color into the pixel shader.
	vout.Uv = Vin.Uv;
	vout.Normal = normalize(Vin.Normal);
	vout.Color = float4(vout.Normal* 0.5f +0.5f, 1.0f);
    vout.PosW=Vin.Pos;
	
    return vout;
}
