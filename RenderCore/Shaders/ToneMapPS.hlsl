#include "PostProcess_BloomUpRS.hlsli"

Texture2D gSceneColor : register(t0);
Texture2D gSunMergeColor : register(t1);

SamplerState gSceneColorSampler : register(s0);

int2 RenderTargetSize : register(b0);


float3 ACESToneMapping(float3 color, float adapted_lum)
{
	const float A = 2.51f;
	const float B = 0.03f;
	const float C = 2.43f;
	const float D = 0.59f;
	const float E = 0.14f;

	color *= adapted_lum;
	return (color * (A * color + B)) / (color * (C * color + D) + E);
}



[RootSignature(PostProcess_BloomUpRootSig)]
float4 main(float4 position : SV_Position) : SV_TARGET
{
	int X = floor(position.x);
	int Y = floor(position.y);

	float2 Tex;
	Tex.x = 1.0f * X / RenderTargetSize[0];
	Tex.y = 1.0f * Y / RenderTargetSize[1];

	float4 SceneColor = gSceneColor.Sample(gSceneColorSampler, Tex);
	float4 BloomColor = gSunMergeColor.Sample(gSceneColorSampler, Tex);

	half3 LinearColor = SceneColor.rgb + BloomColor.rgb;
	
	float4 OutColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
	OutColor.rgb = ACESToneMapping(LinearColor, 1.0f);
	OutColor.a = SceneColor.a;

	return OutColor;
}