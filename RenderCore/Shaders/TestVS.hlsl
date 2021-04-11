#include "RenderCoreRS.hlsli"

cbuffer cbPerObject : register(b0)
{
	float4x4 gWorldViewProj;
	float3 CameraLocation;
};

cbuffer manPassObject : register(b1)
{
	float4x4 View;
	float4x4 Proj;
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
	vout.PosH = mul(float4(Vin.Pos, 1.0f), gWorldViewProj);
	
	// Just pass vertex color into the pixel shader.
	vout.Uv = Vin.Uv;
	vout.Normal = normalize(Vin.Normal);
	vout.Color = float4(vout.Normal* 0.5f +0.5f, 1.0f);
    vout.PosW=Vin.Pos;
	
    return vout;
}
