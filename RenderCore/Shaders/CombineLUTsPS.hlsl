#include "PostProcess_CombineLUTsRS.hlsli"
#include "ACES.hlsli"

int4 RenderTargetSize : register(b0);
//
//static const float3x3 GamutMappingIdentityMatrix = { 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0 };
//
//
//float3x3 OuputGamutMappingMatrix(uint OutputGamut)
//{
//
//	const float3x3 AP1_2_sRGB = mul(XYZ_2_sRGB_MAT, mul(D60_2_D65_CAT, AP1_2_XYZ_MAT));
//	const float3x3 AP1_2_DCI_D65 = mul(XYZ_2_P3D65_MAT, mul(D60_2_D65_CAT, AP1_2_XYZ_MAT));
//	const float3x3 AP1_2_Rec2020 = mul(XYZ_2_Rec2020_MAT, mul(D60_2_D65_CAT, AP1_2_XYZ_MAT));
//
//	if (OutputGamut == 1)
//		return AP1_2_DCI_D65;
//	else if (OutputGamut == 2)
//		return AP1_2_Rec2020;
//	else if (OutputGamut == 3)
//		return AP1_2_AP0_MAT;
//	else if (OutputGamut == 4)
//		return GamutMappingIdentityMatrix;
//	else
//		return AP1_2_sRGB;
//}
//
//float3 LogToLin(float3 LogColor)
//{
//	const float LinearRange = 14;
//	const float LinearGrey = 0.18;
//	const float ExposureGrey = 444;
//
//
//	float3 LinearColor = exp2((LogColor - ExposureGrey / 1023.0) * LinearRange) * LinearGrey;
//
//
//
//
//	return LinearColor;
//}
//
//float CorrelatedColorTemperature(float x, float y)
//{
//	float n = (x - 0.3320) / (0.1858 - y);
//	return -449 * n * n * n + 3525 * n * n - 6823.3 * n + 5520.33;
//}
//
//float Square(float InValue)
//{
//	return InValue* InValue;
//}
//
//float2 PlanckianIsothermal(float Temp, float Tint)
//{
//	float u = (0.860117757f + 1.54118254e-4f * Temp + 1.28641212e-7f * Temp * Temp) / (1.0f + 8.42420235e-4f * Temp + 7.08145163e-7f * Temp * Temp);
//	float v = (0.317398726f + 4.22806245e-5f * Temp + 4.20481691e-8f * Temp * Temp) / (1.0f - 2.89741816e-5f * Temp + 1.61456053e-7f * Temp * Temp);
//
//	float ud = (-1.13758118e9f - 1.91615621e6f * Temp - 1.53177f * Temp * Temp) / Square(1.41213984e6f + 1189.62f * Temp + Temp * Temp);
//	float vd = (1.97471536e9f - 705674.0f * Temp - 308.607f * Temp * Temp) / Square(6.19363586e6f - 179.456f * Temp + Temp * Temp);
//
//	float2 uvd = normalize(float2(u, v));
//
//
//	u += -uvd.y * Tint * 0.05;
//	v += uvd.x * Tint * 0.05;
//
//	float x = 3 * u / (2 * u - 8 * v + 4);
//	float y = 2 * v / (2 * u - 8 * v + 4);
//
//	return float2(x, y);
//}
//
//float2 PlanckianLocusChromaticity(float Temp)
//{
//	float u = (0.860117757f + 1.54118254e-4f * Temp + 1.28641212e-7f * Temp * Temp) / (1.0f + 8.42420235e-4f * Temp + 7.08145163e-7f * Temp * Temp);
//	float v = (0.317398726f + 4.22806245e-5f * Temp + 4.20481691e-8f * Temp * Temp) / (1.0f - 2.89741816e-5f * Temp + 1.61456053e-7f * Temp * Temp);
//
//	float x = 3 * u / (2 * u - 8 * v + 4);
//	float y = 2 * v / (2 * u - 8 * v + 4);
//
//	return float2(x, y);
//}
//
//
//
//
//float2 D_IlluminantChromaticity(float Temp)
//{
//
//
//	Temp *= 1.4388 / 1.438;
//
//	float x = Temp <= 7000 ?
//		0.244063 + (0.09911e3 + (2.9678e6 - 4.6070e9 / Temp) / Temp) / Temp :
//		0.237040 + (0.24748e3 + (1.9018e6 - 2.0064e9 / Temp) / Temp) / Temp;
//
//	float y = -3 * x * x + 2.87 * x - 0.275;
//
//	return float2(x, y);
//}
//
//float WhiteTemp = 6500.0f;
//float WhiteTint = 0.0f;
//
//float BlueCorrection = 0.6f;
//float ExpandGamut = 1.0f;
//
//float3 WhiteBalance(float3 LinearColor)
//{
//	float2 SrcWhiteDaylight = D_IlluminantChromaticity(WhiteTemp);
//	float2 SrcWhitePlankian = PlanckianLocusChromaticity(WhiteTemp);
//
//	float2 SrcWhite = WhiteTemp < 4000 ? SrcWhitePlankian : SrcWhiteDaylight;
//	float2 D65White = float2(0.31270, 0.32900);
//
//	{
//
//		float2 Isothermal = PlanckianIsothermal(WhiteTemp, WhiteTint) - SrcWhitePlankian;
//		SrcWhite += Isothermal;
//	}
//
//	float3x3 WhiteBalanceMat = ChromaticAdaptation(SrcWhite, D65White);
//	WhiteBalanceMat = mul(XYZ_2_sRGB_MAT, mul(WhiteBalanceMat, sRGB_2_XYZ_MAT));
//
//	return mul(WhiteBalanceMat, LinearColor);
//}
//
//
//float3 ColorCorrect(float3 WorkingColor,
//	float4 ColorSaturation,
//	float4 ColorContrast,
//	float4 ColorGamma,
//	float4 ColorGain,
//	float4 ColorOffset)
//{
//
//	float Luma = dot(WorkingColor, AP1_RGB2Y);
//	WorkingColor = max(0, lerp(Luma.xxx, WorkingColor, ColorSaturation.xyz * ColorSaturation.w));
//	WorkingColor = pow(WorkingColor * (1.0 / 0.18), ColorContrast.xyz * ColorContrast.w) * 0.18;
//	WorkingColor = pow(WorkingColor, 1.0 / (ColorGamma.xyz * ColorGamma.w));
//	WorkingColor = WorkingColor * (ColorGain.xyz * ColorGain.w) + (ColorOffset.xyz + ColorOffset.w);
//	return WorkingColor;
//}
//
//
//
//float3 ColorCorrectAll(float3 WorkingColor)
//{
//	float Luma = dot(WorkingColor, AP1_RGB2Y);
//
//
//	float3 CCColorShadows = ColorCorrect(WorkingColor,
//		ColorSaturationShadows * ColorSaturation,
//		ColorContrastShadows * ColorContrast,
//		ColorGammaShadows * ColorGamma,
//		ColorGainShadows * ColorGain,
//		ColorOffsetShadows + ColorOffset);
//	float CCWeightShadows = 1 - smoothstep(0, ColorCorrectionShadowsMax, Luma);
//
//
//	float3 CCColorHighlights = ColorCorrect(WorkingColor,
//		ColorSaturationHighlights * ColorSaturation,
//		ColorContrastHighlights * ColorContrast,
//		ColorGammaHighlights * ColorGamma,
//		ColorGainHighlights * ColorGain,
//		ColorOffsetHighlights + ColorOffset);
//	float CCWeightHighlights = smoothstep(ColorCorrectionHighlightsMin, 1, Luma);
//
//
//	float3 CCColorMidtones = ColorCorrect(WorkingColor,
//		ColorSaturationMidtones * ColorSaturation,
//		ColorContrastMidtones * ColorContrast,
//		ColorGammaMidtones * ColorGamma,
//		ColorGainMidtones * ColorGain,
//		ColorOffsetMidtones + ColorOffset);
//	float CCWeightMidtones = 1 - CCWeightShadows - CCWeightHighlights;
//
//
//	float3 WorkingColorSMH = CCColorShadows * CCWeightShadows + CCColorMidtones * CCWeightMidtones + CCColorHighlights * CCWeightHighlights;
//
//	return WorkingColorSMH;
//}

[RootSignature(PostProcess_CombineLUTsRootSig)]
float4 main(float4 position : SV_Position) : SV_TARGET
{
	
	int LUTSize = RenderTargetSize[1];

	float U = 1.0f * position.x / (LUTSize * LUTSize);
	float V = 1.0f * position.y / LUTSize;

	float4 Neutral;
	{
		float2 UV = float2(U, V);


		UV -= float2(0.49999f / (LUTSize * LUTSize), 0.49999f / LUTSize);

		float Scale = 1.0f * LUTSize / (LUTSize - 1);

		float3 RGB;

		RGB.r = frac(UV.x * LUTSize);
		RGB.b = UV.x - RGB.r / LUTSize;
		RGB.g = UV.y;

		Neutral = float4(RGB * Scale, 0);
	}

	//float4 OutColor = 0;


	//const float3x3 sRGB_2_AP1 = mul(XYZ_2_AP1_MAT, mul(D65_2_D60_CAT, sRGB_2_XYZ_MAT));
	//const float3x3 AP1_2_sRGB = mul(XYZ_2_sRGB_MAT, mul(D60_2_D65_CAT, AP1_2_XYZ_MAT));

	//const float3x3 AP0_2_AP1 = mul(XYZ_2_AP1_MAT, AP0_2_XYZ_MAT);
	//const float3x3 AP1_2_AP0 = mul(XYZ_2_AP0_MAT, AP1_2_XYZ_MAT);

	//const float3x3 AP1_2_Output = OuputGamutMappingMatrix(0);

	//float3 LUTEncodedColor = Neutral.rgb;
	//float3 LinearColor;

	//LinearColor = LogToLin(LUTEncodedColor) - LogToLin(0);

	//float3 BalancedColor = WhiteBalance(LinearColor);
	//float3 ColorAP1 = mul(sRGB_2_AP1, BalancedColor);


	//float LumaAP1 = dot(ColorAP1, AP1_RGB2Y);
	//float3 ChromaAP1 = ColorAP1 / LumaAP1;

	//float ChromaDistSqr = dot(ChromaAP1 - 1, ChromaAP1 - 1);
	//float ExpandAmount = (1 - exp2(-4 * ChromaDistSqr)) * (1 - exp2(-4 * ExpandGamut * LumaAP1 * LumaAP1));


	//const float3x3 Wide_2_XYZ_MAT =
	//{
	//	0.5441691, 0.2395926, 0.1666943,
	//	0.2394656, 0.7021530, 0.0583814,
	//	-0.0023439, 0.0361834, 1.0552183,
	//};

	//const float3x3 Wide_2_AP1 = mul(XYZ_2_AP1_MAT, Wide_2_XYZ_MAT);
	//const float3x3 ExpandMat = mul(Wide_2_AP1, AP1_2_sRGB);

	//float3 ColorExpand = mul(ExpandMat, ColorAP1);
	//ColorAP1 = lerp(ColorAP1, ColorExpand, ExpandAmount);

	//ColorAP1 = ColorCorrectAll(ColorAP1);


	//float3 GradedColor = mul(AP1_2_sRGB, ColorAP1);

	//const float3x3 BlueCorrect =
	//{
	//	0.9404372683, -0.0183068787, 0.0778696104,
	//	0.0083786969, 0.8286599939, 0.1629613092,
	//	0.0005471261, -0.0008833746, 1.0003362486
	//};
	//const float3x3 BlueCorrectInv =
	//{
	//	1.06318, 0.0233956, -0.0865726,
	//	-0.0106337, 1.20632, -0.19569,
	//	-0.000590887, 0.00105248, 0.999538
	//};
	//const float3x3 BlueCorrectAP1 = mul(AP0_2_AP1, mul(BlueCorrect, AP1_2_AP0));
	//const float3x3 BlueCorrectInvAP1 = mul(AP0_2_AP1, mul(BlueCorrectInv, AP1_2_AP0));


	//ColorAP1 = lerp(ColorAP1, mul(BlueCorrectAP1, ColorAP1), BlueCorrection);


	//ColorAP1 = FilmToneMap(ColorAP1);


	//ColorAP1 = lerp(ColorAP1, mul(BlueCorrectInvAP1, ColorAP1), BlueCorrection);

	//float3 FilmColor = max(0, mul(AP1_2_sRGB, ColorAP1));


	//FilmColor = ColorCorrection(FilmColor);


	//float3 FilmColorNoGamma = lerp(FilmColor * ColorScale, OverlayColor.rgb, OverlayColor.a);

	//GradedColor = lerp(GradedColor * ColorScale, OverlayColor.rgb, OverlayColor.a);


	//FilmColor = pow(max(0, FilmColorNoGamma), InverseGamma.y);

	//half3 OutDeviceColor = 0;

	//float3 OutputGamutColor = FilmColor;
	//OutDeviceColor = LinearToSrgb(OutputGamutColor);

	//OutColor.rgb = OutDeviceColor / 1.05;
	//OutColor.a = 0;

	//return OutColor;


	return Neutral;
}