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


[RootSignature(PostProcess_BloomUpRootSig)]
float4 main(float4 position : SV_Position) : SV_TARGET
{
	float4 OutColor = float4(0.0f, 0.0f, 0.0f, 0.0f);


	float BloomWeightScalar = 1.0f / RenderTargetSize[2];
	float BloomWeightScalar1 = 1.0f / RenderTargetSize[3];

	float BloomUpScale = 1.32f;

	int X = floor(position.x);
	int Y = floor(position.y);


	float2 Tex;
	Tex.x = 1.0f * X / RenderTargetSize[0];
	Tex.y = 1.0f * Y / RenderTargetSize[1];

	float DeltaU = 1.0f / RenderTargetSize[0];
	float DeltaV = 1.0f / RenderTargetSize[1];
	float2 DeltaUV = float2(DeltaU, DeltaV);

	float Start = 2.0 / 7.0;
	float4 Color0 = gBloomUp.Sample(gBloomInputSampler, Tex + DeltaUV * BloomUpScale * Circle(Start, 7.0, 0.0f));
	float4 Color1 = gBloomUp.Sample(gBloomInputSampler, Tex + DeltaUV * BloomUpScale * Circle(Start, 7.0, 1.0f));
	float4 Color2 = gBloomUp.Sample(gBloomInputSampler, Tex + DeltaUV * BloomUpScale * Circle(Start, 7.0, 2.0f));
	float4 Color3 = gBloomUp.Sample(gBloomInputSampler, Tex + DeltaUV * BloomUpScale * Circle(Start, 7.0, 3.0f));
	float4 Color4 = gBloomUp.Sample(gBloomInputSampler, Tex + DeltaUV * BloomUpScale * Circle(Start, 7.0, 4.0f));
	float4 Color5 = gBloomUp.Sample(gBloomInputSampler, Tex + DeltaUV * BloomUpScale * Circle(Start, 7.0, 5.0f));
	float4 Color6 = gBloomUp.Sample(gBloomInputSampler, Tex + DeltaUV * BloomUpScale * Circle(Start, 7.0, 6.0f));
	float4 Color7 = gBloomUp.Sample(gBloomInputSampler, Tex);

	float4 Color8 = gBloomDown.Sample(gBloomInputSampler, Tex + DeltaUV * BloomUpScale * Circle(Start, 7.0, 0.0f));
	float4 Color9 = gBloomDown.Sample(gBloomInputSampler, Tex + DeltaUV * BloomUpScale * Circle(Start, 7.0, 1.0f));
	float4 Color10 = gBloomDown.Sample(gBloomInputSampler, Tex + DeltaUV * BloomUpScale * Circle(Start, 7.0, 2.0f));
	float4 Color11 = gBloomDown.Sample(gBloomInputSampler, Tex + DeltaUV * BloomUpScale * Circle(Start, 7.0, 3.0f));
	float4 Color12 = gBloomDown.Sample(gBloomInputSampler, Tex + DeltaUV * BloomUpScale * Circle(Start, 7.0, 4.0f));
	float4 Color13 = gBloomDown.Sample(gBloomInputSampler, Tex + DeltaUV * BloomUpScale * Circle(Start, 7.0, 5.0f));
	float4 Color14 = gBloomDown.Sample(gBloomInputSampler, Tex + DeltaUV * BloomUpScale * Circle(Start, 7.0, 6.0f));
	float4 Color15 = gBloomDown.Sample(gBloomInputSampler, Tex);



	float4 BloomWight = float4(BloomWeightScalar, BloomWeightScalar, BloomWeightScalar, 0.0f);
	float4 BloomWight1 = float4(BloomWeightScalar1, BloomWeightScalar1, BloomWeightScalar1, 0.0f);

	OutColor = (Color0 + Color1 + Color2 + Color3 + Color4 + Color5 + Color6 + Color7) * BloomWight +
		(Color8 + Color9 + Color10 + Color11 + Color12 + Color13 + Color14 + Color15) * BloomWight1;
	OutColor.a = 0.0f;


	return OutColor;
}