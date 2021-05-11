#include "ScreenRS.hlsli"

Texture2D gSceneColor : register(t0);
SamplerState gSceneColorSampler : register(s0);

int2 RenderTargetSize : register(b0);

float Luminance(float3 InColor)
{
	return dot(InColor, float3(0.3f, 0.59f, 0.11f));
}

[RootSignature(PostProcess_RootSig)]
float4 main(float4 position : SV_Position) : SV_TARGET
{
	//todo: exposed in C++
	const float OneOverPreExposure = 1.0f;
	const float PreExposure = 1.0f;
	const float BloomThreshold = -1.0f;

	int X = floor(position.x);
	int Y = floor(position.y);

	float Width = RenderTargetSize[0] * 0.25f;
	float Height = RenderTargetSize[1] * 0.25f;

	float DeltaU = 1.0f / RenderTargetSize[0];
	float DeltaV = 1.0f / RenderTargetSize[1];

	float2 Tex;
	Tex.x = 1.0f * X / Width;
	Tex.y = 1.0f * Y / Height;

	float4 Color0 = gSceneColor.Sample(gSceneColorSampler, Tex + float2(-DeltaU, -DeltaV));
	float4 Color1 = gSceneColor.Sample(gSceneColorSampler, Tex + float2(+DeltaU, -DeltaV));
	float4 Color2 = gSceneColor.Sample(gSceneColorSampler, Tex + float2(-DeltaU, +DeltaV));
	float4 Color3 = gSceneColor.Sample(gSceneColorSampler, Tex + float2(+DeltaU, +DeltaV));

	float4 AvailableColor = Color0 * 0.25f + Color1 * 0.25f + Color2 * 0.25f + Color3 * 0.25f;
	
	AvailableColor.rgb = max(AvailableColor.rgb, 0) * OneOverPreExposure;

	float TotalLuminance = Luminance(AvailableColor.rgb);
	float BloomLuminance = TotalLuminance - BloomThreshold;
	float Amount = saturate(BloomLuminance * 0.5f);

	float4 OutColor;
	OutColor.rgb = AvailableColor.rgb;
	OutColor.rgb *= Amount;
	OutColor.a = 0.0f;

	return OutColor;
}