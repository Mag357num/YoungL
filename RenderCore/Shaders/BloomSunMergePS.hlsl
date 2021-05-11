#include "PostProcess_BloomUpRS.hlsli"

Texture2D gBloomUp : register(t0);
Texture2D gBloomDown : register(t1);

SamplerState gBloomInputSampler : register(s0);

int4 RenderTargetSize : register(b0);

float2 Circle(float Start, float Points, float Point)
{
	float Radians = (2.0f * 3.141592f * (1.0f / Points)) * (Start + Point);
	return float2(cos(Radians), sin(Radians));
}

float2 SunShaftPosToUV(float2 Pos)
{

	return Pos.xy * float2(0.5, -0.5) + 0.5;
}

float2 SunPos()
{
	float4 LightShaftCenter = float4(0.0f, 0.0f, 0.0f, 1.0f);

	return LightShaftCenter.xy;
}

float2 SunShaftRect(float2 InPosition, float amount)
{
	float2 center = SunPos();
	return SunShaftPosToUV(lerp(center, InPosition, amount));
}

float2 VignetteSpace(float2 Pos, float AspectRatio)
{

	float Scale = sqrt(2.0) / sqrt(1.0 + AspectRatio * AspectRatio);
	return Pos * float2(1.0, AspectRatio) * Scale;
}

float Square(float x)
{
	return x * x;
}

float ComputeVignetteMask(float2 VignetteCircleSpacePos, float Intensity)
{
	VignetteCircleSpacePos *= Intensity;
	float Tan2Angle = dot(VignetteCircleSpacePos, VignetteCircleSpacePos);
	float Cos4Angle = Square(rcp(Tan2Angle + 1));
	return Cos4Angle;
}

[RootSignature(PostProcess_BloomUpRootSig)]
float4 main(float4 position : SV_Position) : SV_TARGET
{
	float4 OutColor = float4(0.0f, 0.0f, 0.0f, 0.0f);

	float ScaleUV = 0.66f / 2.0f;
	float4 BloomColor1 = float4(0.5016f, 0.5016f, 0.5016f, 0.0f);

	int X = floor(position.x);
	int Y = floor(position.y);


	float2 Tex;
	Tex.x = 1.0f * X / RenderTargetSize[0];
	Tex.y = 1.0f * Y / RenderTargetSize[1];


	float DeltaU = 1.0f / RenderTargetSize[0];
	float DeltaV = 1.0f / RenderTargetSize[1];
	float2 DeltaUV = float2(DeltaU, DeltaV);

	float Start = 2.0f / 6.0f;
	float4 Color0 = gBloomDown.Sample(gBloomInputSampler, Tex + DeltaUV * ScaleUV * Circle(Start, 6.0, 0.0f));
	float4 Color1 = gBloomDown.Sample(gBloomInputSampler, Tex + DeltaUV * ScaleUV * Circle(Start, 6.0, 1.0f));
	float4 Color2 = gBloomDown.Sample(gBloomInputSampler, Tex + DeltaUV * ScaleUV * Circle(Start, 6.0, 2.0f));
	float4 Color3 = gBloomDown.Sample(gBloomInputSampler, Tex + DeltaUV * ScaleUV * Circle(Start, 6.0, 3.0f));
	float4 Color4 = gBloomDown.Sample(gBloomInputSampler, Tex + DeltaUV * ScaleUV * Circle(Start, 6.0, 4.0f));
	float4 Color5 = gBloomDown.Sample(gBloomInputSampler, Tex + DeltaUV * ScaleUV * Circle(Start, 6.0, 5.0f));
	float4 Color6 = gBloomDown.Sample(gBloomInputSampler, Tex);
	
	float ScaleColor1 = 1.0f / 7.0f;
	float ScaleColor2 = 1.0f / 7.0f;
	float BloomColor = Color6 * ScaleColor1 +
		Color0 * ScaleColor2 +
		Color1 * ScaleColor2 +
		Color2 * ScaleColor2 +
		Color3 * ScaleColor2 +
		Color4 * ScaleColor2 +
		Color4 * ScaleColor2 * rcp(ScaleColor1 * 1.0f + ScaleColor2 * 6.0f);

	OutColor.rgb = gBloomUp.Sample(gBloomInputSampler, Tex).rgb;

	float ScaleColor3 = 1.0f / 5.0f;
	OutColor.rgb *= ScaleColor3;


	OutColor.rgb += (BloomColor * ScaleColor3 * BloomColor1).rgb;

	float SunColorVignetteIntensity = 0.4f;

	float2 SV_Position = Tex * 2.0f - 1.0f;
	float2 VignetteSpacePos = VignetteSpace(SV_Position, 1.0f);
	float Vignette = ComputeVignetteMask(VignetteSpacePos, SunColorVignetteIntensity);
	OutColor.a = Vignette;
	OutColor.rgb *= OutColor.a;
	//calc sun shaft
	//float SunShaftUV0 = SunShaftRect(SV_Position.xy, 1.0f - (31.0f / 32.0f) * 0.25f);
	//float SunShaftUV1 = SunShaftRect(SV_Position.xy, 1.0f - (27.0f / 32.0f) * 0.25f);
	//float SunShaftUV2 = SunShaftRect(SV_Position.xy, 1.0f - (23.0f / 32.0f) * 0.25f);
	//float SunShaftUV3 = SunShaftRect(SV_Position.xy, 1.0f - (19.0f / 32.0f) * 0.25f);
	//float SunShaftUV4 = SunShaftRect(SV_Position.xy, 1.0f - (15.0f / 32.0f) * 0.25f);
	//float SunShaftUV5 = SunShaftRect(SV_Position.xy, 1.0f - (11.0f / 32.0f) * 0.25f);
	//float SunShaftUV6 = SunShaftRect(SV_Position.xy, 1.0f - (7.0f / 32.0f) * 0.25f);
	//float SunShaftUV7 = float2(0.0f, 0.0f);

	return OutColor;
}