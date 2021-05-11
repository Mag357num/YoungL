#include "ScreenRS.hlsli"

Texture2D gBloomInput : register(t0);
SamplerState gBloomInputSampler : register(s0);

int2 RenderTargetSize : register(b0);

float2 Circle(float Start, float Points, float Point)
{
	float Radians = (2.0f * 3.141592f * (1.0f / Points)) * (Start + Point);
	return float2(cos(Radians), sin(Radians));
}

[RootSignature(PostProcess_RootSig)]
float4 main(float4 position : SV_Position) : SV_TARGET
{
	float BloomDownScale = 2.5f;

	int X = floor(position.x);
	int Y = floor(position.y);

	float Width = RenderTargetSize[0] * 0.5f;
	float Height = RenderTargetSize[1] * 0.5f;

	float DeltaU = 1.0f / RenderTargetSize[0];
	float DeltaV = 1.0f / RenderTargetSize[1];

	float2 Tex;
	Tex.x = 1.0f * X / Width;
	Tex.y = 1.0f * Y / Height;


	float2 DeltaUV = float2(DeltaU, DeltaV);

	float StartRaduas = 2.0f / 14.0f;
	float4 Color0 = gBloomInput.Sample(gBloomInputSampler, Tex + DeltaUV * BloomDownScale * Circle(StartRaduas, 14.0f, 0.0f));
	float4 Color1 = gBloomInput.Sample(gBloomInputSampler, Tex + DeltaUV * BloomDownScale * Circle(StartRaduas, 14.0f, 1.0f));
	float4 Color2 = gBloomInput.Sample(gBloomInputSampler, Tex + DeltaUV * BloomDownScale * Circle(StartRaduas, 14.0f, 2.0f));
	float4 Color3 = gBloomInput.Sample(gBloomInputSampler, Tex + DeltaUV * BloomDownScale * Circle(StartRaduas, 14.0f, 3.0f));
	float4 Color4 = gBloomInput.Sample(gBloomInputSampler, Tex + DeltaUV * BloomDownScale * Circle(StartRaduas, 14.0f, 4.0f));
	float4 Color5 = gBloomInput.Sample(gBloomInputSampler, Tex + DeltaUV * BloomDownScale * Circle(StartRaduas, 14.0f, 5.0f));
	float4 Color6 = gBloomInput.Sample(gBloomInputSampler, Tex + DeltaUV * BloomDownScale * Circle(StartRaduas, 14.0f, 6.0f));
	float4 Color7 = gBloomInput.Sample(gBloomInputSampler, Tex + DeltaUV * BloomDownScale * Circle(StartRaduas, 14.0f, 7.0f));
	float4 Color8 = gBloomInput.Sample(gBloomInputSampler, Tex + DeltaUV * BloomDownScale * Circle(StartRaduas, 14.0f, 8.0f));
	float4 Color9 = gBloomInput.Sample(gBloomInputSampler, Tex + DeltaUV * BloomDownScale * Circle(StartRaduas, 14.0f, 9.0f));
	float4 Color10 = gBloomInput.Sample(gBloomInputSampler, Tex + DeltaUV * BloomDownScale * Circle(StartRaduas, 14.0f, 10.0f));
	float4 Color11 = gBloomInput.Sample(gBloomInputSampler, Tex + DeltaUV * BloomDownScale * Circle(StartRaduas, 14.0f, 11.0f));
	float4 Color12 = gBloomInput.Sample(gBloomInputSampler, Tex + DeltaUV * BloomDownScale * Circle(StartRaduas, 14.0f, 12.0f));
	float4 Color13 = gBloomInput.Sample(gBloomInputSampler, Tex + DeltaUV * BloomDownScale * Circle(StartRaduas, 14.0f, 13.0f));

	float4 Color = gBloomInput.Sample(gBloomInputSampler, Tex);

	float Weight = 1.0f / 15.0f;
	
	Color = Weight * (Color + Color0 + Color1 + Color2 + Color3
					+ Color4 + Color5 + Color6 + Color7
					+ Color8 + Color9 + Color10 + Color11 + Color12 + Color13);

	return Color;
}