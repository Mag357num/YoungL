#include "ScreenRS.hlsli"

Texture2D gSceneColor : register(t0);
SamplerState gSceneColorSampler : register(s0);

int2 RenderTargetSize : register(b0);

[RootSignature(PostProcess_RootSig)]
float4 main(float4 position : SV_Position) : SV_TARGET
{
	int X = floor(position.x);
	int Y = floor(position.y);

	float2 Tex;
	Tex.x = 1.0f * X / RenderTargetSize[0];
	Tex.y = 1.0f * Y / RenderTargetSize[1];

	float4 Color = gSceneColor.Sample(gSceneColorSampler, Tex);
	//float4 Color = gSceneColor.Load(int3(X, Y, 0));
	return Color;
}