#include "SkinnedMeshRS.hlsli"
#include "Common.hlsli"

cbuffer cbPerObject : register(b0)
{
	float4x4 ObjectWorld;

	float3 Fresnel0;
	float Shiness;
	float3 AmbientLight;
};

cbuffer cbBoneTranforms : register(b2)
{
	float4x4 BoneTransforms[96];
}

cbuffer mainPassObject : register(b1)
{
	float4x4 ViewProj;
	float4x4 LightViewProj;
	float4 CamLocation;

	//for global directional lighting
	float4 LightDirection;
	float4 LightStrength;
};

[RootSignature(SkinnedMesh_RootSig)]
VertexOut main(SkinnedVertexIn Vin)
{
	//multiply bone transform before to world
	float4 Weights = float4(0.0f, 0.0f, 0.0f, 0.0f);
	Weights[0] = Vin.BoneWeight[0];
	Weights[1] = Vin.BoneWeight[1];
	Weights[2] = Vin.BoneWeight[2];
	Weights[3] = 1.0f -(Weights[0]+ Weights[1]+ Weights[2]);

	float3 PosByBone = float3(0.0f, 0.0f, 0.0f);
	float3 NormalByBone = float3(0.0f, 0.0f, 0.0f);

	for (int i = 0; i < 4; ++i)
	{
		PosByBone += Weights[i] * mul(float4(Vin.Pos, 1.0f), BoneTransforms[Vin.BoneIndex[i]]).xyz;
		NormalByBone += Weights[i] * mul(float4(Vin.Normal, 1.0f), BoneTransforms[Vin.BoneIndex[i]]).xyz;
	}

	//set vin.pos 
	Vin.Pos = PosByBone;
	Vin.Normal = NormalByBone;

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
