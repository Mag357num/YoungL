#ifndef NUM_DIR_LIGHTS
	#define NUM_DIR_LIGHTS 3
#endif

#ifndef NUM_SPOT_LIGHTS
	#define NUM_SPOT_LIGHTS 0
#endif

#ifndef NUM_POINT_LIGHTS
	#define NUM_POINT_LIGHTS 0
#endif

#include "LitWav_LightingUtil.hlsl"

struct InstanceData
{
	float4x4 World;
	float4x4 TexTransform;
	uint MaterialIndex;
	uint InstPad0;
	uint InstPad1;
	uint InstPad2;
};

struct MaterialData
{
	float4 DiffuseAlbedo;
	float3 FresnelR0;
	float3 Roughtness;
	float4x4 MatTransform;
	uint DiffuseMapIndex;
	uint MatPad0;
	uint MatPad1;
	uint MatPad2;
};

Texture2D gDiffuseMap[7]:register(t0);

// Put in space1, so the texture array does not overlap with these resources.  
// The texture array will occupy registers t0, t1, ..., t6 in space0. 
StructuredBuffer<InstanceData> gInstantceData:register(t0, space1);
StructuredBuffer<MaterialData> gMaterialData:register(t1, space1);

SamplerState gsamPointWrap			:register(s0);
SamplerState gsamPointClamp			: register(s1);
SamplerState gsamLinearWrap			: register(s2);
SamplerState gsamLinearClamp		: register(s3);
SamplerState gsamAnisotropicWrap	: register(s4);
SamplerState gsamAnisotropicClamp	: register(s5);

struct VertexIn
{
	float3 PosL		:POSITION;
	float3 Normal	:NORMAL;
	float2 TexC		:TEXCOORD;
};

struct VertexOut
{
	float4 PosH		:SV_POSITION;
	float3 PosW		:POSITION;
	float3 NormalW	:NORMAL;
	float2 TexC		:TEXCOORD;

	//nointerpolation is used so the index is not interpolated
	//across the triangle
	nointerpolation uint MatIndex	:MATINDEX;
};

VertexOut VS(VeretexIn vin, uint instanceID:SV_InstanceID)
{
	VertexOut vout = (VertexOut)0.0f;

	//fetch the instance data
	InstanceData instData = gInstantceData[instanceID];
	float4x4 world = instData.World;
	float4x4 texTransform = instData.TexTransform;
	uint matIndex = instData.MaterialIndex;

	vout.MatIndex = matIndex;
	MaterialData matData = gMaterialData[matIndex];

	//transform to world space
	float PosW = mul(float4(vin.PosL, 1.0f), world);
	vout.PosW = posW.xyz;

	//assume nonuniform scaling; otherwise, need to use inverse-transpose of world matrix.
	vout.NormalW = mul(vin.Normal, (float3x3)world);

	vout.PosH = mul(posW, gViewProj);

	float4 texC = mul(float4(vin.TexC, 0.0f, 1.0f), texTransform);
	vout.TexC = mul(texC, matData.MatTransform).xy;

	return vout;
}
