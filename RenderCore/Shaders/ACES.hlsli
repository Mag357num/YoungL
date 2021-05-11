



















































static const float PI = 3.14159265f;


static const float3x3 AP0_2_XYZ_MAT =
{
	0.9525523959, 0.0000000000, 0.0000936786,
	0.3439664498, 0.7281660966,-0.0721325464,
	0.0000000000, 0.0000000000, 1.0088251844,
};

static const float3x3 XYZ_2_AP0_MAT =
{
	 1.0498110175, 0.0000000000,-0.0000974845,
	-0.4959030231, 1.3733130458, 0.0982400361,
	 0.0000000000, 0.0000000000, 0.9912520182,
};

static const float3x3 AP1_2_XYZ_MAT =
{
	 0.6624541811, 0.1340042065, 0.1561876870,
	 0.2722287168, 0.6740817658, 0.0536895174,
	-0.0055746495, 0.0040607335, 1.0103391003,
};

static const float3x3 XYZ_2_AP1_MAT =
{
	 1.6410233797, -0.3248032942, -0.2364246952,
	-0.6636628587, 1.6153315917, 0.0167563477,
	 0.0117218943, -0.0082844420, 0.9883948585,
};

static const float3x3 AP0_2_AP1_MAT =
{
	 1.4514393161, -0.2365107469, -0.2149285693,
	-0.0765537734, 1.1762296998, -0.0996759264,
	 0.0083161484, -0.0060324498, 0.9977163014,
};

static const float3x3 AP1_2_AP0_MAT =
{
	 0.6954522414, 0.1406786965, 0.1638690622,
	 0.0447945634, 0.8596711185, 0.0955343182,
	-0.0055258826, 0.0040252103, 1.0015006723,
};

static const float3 AP1_RGB2Y =
{
	0.2722287168,
	0.6740817658,
	0.0536895174,
};


static const float3x3 XYZ_2_sRGB_MAT =
{
	 3.2409699419, -1.5373831776, -0.4986107603,
	-0.9692436363, 1.8759675015, 0.0415550574,
	 0.0556300797, -0.2039769589, 1.0569715142,
};

static const float3x3 sRGB_2_XYZ_MAT =
{
	0.4124564, 0.3575761, 0.1804375,
	0.2126729, 0.7151522, 0.0721750,
	0.0193339, 0.1191920, 0.9503041,
};


static const float3x3 XYZ_2_Rec2020_MAT =
{
	 1.7166084, -0.3556621, -0.2533601,
	-0.6666829, 1.6164776, 0.0157685,
	 0.0176422, -0.0427763, 0.94222867
};

static const float3x3 Rec2020_2_XYZ_MAT =
{
	0.6369736, 0.1446172, 0.1688585,
	0.2627066, 0.6779996, 0.0592938,
	0.0000000, 0.0280728, 1.0608437
};


static const float3x3 XYZ_2_P3D65_MAT =
{
	 2.4933963, -0.9313459, -0.4026945,
	-0.8294868, 1.7626597, 0.0236246,
	 0.0358507, -0.0761827, 0.9570140
};

static const float3x3 P3D65_2_XYZ_MAT =
{
	0.4865906, 0.2656683, 0.1981905,
	0.2289838, 0.6917402, 0.0792762,
	0.0000000, 0.0451135, 1.0438031
};


static const float3x3 D65_2_D60_CAT =
{
	 1.01303, 0.00610531, -0.014971,
	 0.00769823, 0.998165, -0.00503203,
	-0.00284131, 0.00468516, 0.924507,
};

static const float3x3 D60_2_D65_CAT =
{
	 0.987224, -0.00611327, 0.0159533,
	-0.00759836, 1.00186, 0.00533002,
	 0.00307257, -0.00509595, 1.08168,
};

static const float HALF_MAX = 65504.0;

float rgb_2_saturation(float3 rgb)
{
	float minrgb = min(min(rgb.r, rgb.g), rgb.b);
	float maxrgb = max(max(rgb.r, rgb.g), rgb.b);
	return (max(maxrgb, 1e-10) - max(minrgb, 1e-10)) / max(maxrgb, 1e-2);
}

float glow_fwd(float ycIn, float glowGainIn, float glowMid)
{
	float glowGainOut;

	if (ycIn <= 2. / 3. * glowMid) {
		glowGainOut = glowGainIn;
	}
	else if (ycIn >= 2 * glowMid) {
		glowGainOut = 0;
	}
	else {
		glowGainOut = glowGainIn * (glowMid / ycIn - 0.5);
	}

	return glowGainOut;
}

float glow_inv(float ycOut, float glowGainIn, float glowMid)
{
	float glowGainOut;

	if (ycOut <= ((1 + glowGainIn) * 2. / 3. * glowMid)) {
		glowGainOut = -glowGainIn / (1 + glowGainIn);
	}
	else if (ycOut >= (2. * glowMid)) {
		glowGainOut = 0.;
	}
	else {
		glowGainOut = glowGainIn * (glowMid / ycOut - 1. / 2.) / (glowGainIn / 2. - 1.);
	}

	return glowGainOut;
}


float sigmoid_shaper(float x)
{


	float t = max(1 - abs(0.5 * x), 0);
	float y = 1 + sign(x) * (1 - t * t);
	return 0.5 * y;
}



float cubic_basis_shaper
(
	float x,
	float w
)
{


	float M[4][4] =
	{
		{ -1. / 6, 3. / 6, -3. / 6, 1. / 6 },
		{ 3. / 6, -6. / 6, 3. / 6, 0. / 6 },
		{ -3. / 6, 0. / 6, 3. / 6, 0. / 6 },
		{ 1. / 6, 4. / 6, 1. / 6, 0. / 6 }
	};

	float knots[5] = { -0.5 * w, -0.25 * w, 0, 0.25 * w, 0.5 * w };

	float y = 0;
	if ((x > knots[0]) && (x < knots[4]))
	{
		float knot_coord = (x - knots[0]) * 4.0 / w;
		int j = knot_coord;
		float t = knot_coord - j;

		float monomials[4] = { t * t * t, t * t, t, 1.0 };


		if (j == 3) {
			y = monomials[0] * M[0][0] + monomials[1] * M[1][0] +
				monomials[2] * M[2][0] + monomials[3] * M[3][0];
		}
		else if (j == 2) {
			y = monomials[0] * M[0][1] + monomials[1] * M[1][1] +
				monomials[2] * M[2][1] + monomials[3] * M[3][1];
		}
		else if (j == 1) {
			y = monomials[0] * M[0][2] + monomials[1] * M[1][2] +
				monomials[2] * M[2][2] + monomials[3] * M[3][2];
		}
		else if (j == 0) {
			y = monomials[0] * M[0][3] + monomials[1] * M[1][3] +
				monomials[2] * M[2][3] + monomials[3] * M[3][3];
		}
		else {
			y = 0.0;
		}
	}

	return y * 1.5;
}

float center_hue(float hue, float centerH)
{
	float hueCentered = hue - centerH;
	if (hueCentered < -180.)
		hueCentered += 360;
	else if (hueCentered > 180.)
		hueCentered -= 360;
	return hueCentered;
}



static const float3x3 M =
{
	{ 0.5, -1.0, 0.5 },
	{ -1.0, 1.0, 0.5 },
	{ 0.5, 0.0, 0.0 }
};

struct SegmentedSplineParams_c5
{
	float coefsLow[6];
	float coefsHigh[6];
	float2 minPoint;
	float2 midPoint;
	float2 maxPoint;
	float slopeLow;
	float slopeHigh;
};

struct SegmentedSplineParams_c9
{
	float coefsLow[10];
	float coefsHigh[10];
	float2 minPoint;
	float2 midPoint;
	float2 maxPoint;
	float slopeLow;
	float slopeHigh;
};

float segmented_spline_c5_fwd(float x)
{

	const SegmentedSplineParams_c5 C =
	{

		{ -4.0000000000, -4.0000000000, -3.1573765773, -0.4852499958, 1.8477324706, 1.8477324706 },

		{ -0.7185482425, 2.0810307172, 3.6681241237, 4.0000000000, 4.0000000000, 4.0000000000 },
		{ 0.18 * exp2(-15.0), 0.0001},
		{ 0.18, 4.8},
		{ 0.18 * exp2(18.0), 10000.},
		0.0,
		0.0
	};

	const int N_KNOTS_LOW = 4;
	const int N_KNOTS_HIGH = 4;



	float xCheck = x <= 0 ? exp2(-14.0) : x;

	float logx = log10(xCheck);
	float logy;

	if (logx <= log10(C.minPoint.x))
	{
		logy = logx * C.slopeLow + (log10(C.minPoint.y) - C.slopeLow * log10(C.minPoint.x));
	}
	else if ((logx > log10(C.minPoint.x)) && (logx < log10(C.midPoint.x)))
	{
		float knot_coord = (N_KNOTS_LOW - 1) * (logx - log10(C.minPoint.x)) / (log10(C.midPoint.x) - log10(C.minPoint.x));
		int j = knot_coord;
		float t = knot_coord - j;

		float3 cf = { C.coefsLow[j], C.coefsLow[j + 1], C.coefsLow[j + 2] };

		float3 monomials = { t * t, t, 1.0 };
		logy = dot(monomials, mul(cf, M));
	}
	else if ((logx >= log10(C.midPoint.x)) && (logx < log10(C.maxPoint.x)))
	{
		float knot_coord = (N_KNOTS_HIGH - 1) * (logx - log10(C.midPoint.x)) / (log10(C.maxPoint.x) - log10(C.midPoint.x));
		int j = knot_coord;
		float t = knot_coord - j;

		float3 cf = { C.coefsHigh[j], C.coefsHigh[j + 1], C.coefsHigh[j + 2] };

		float3 monomials = { t * t, t, 1.0 };
		logy = dot(monomials, mul(cf, M));
	}
	else
	{
		logy = logx * C.slopeHigh + (log10(C.maxPoint.y) - C.slopeHigh * log10(C.maxPoint.x));
	}

	return pow(10, logy);
}

float segmented_spline_c5_rev(float y)
{

	const SegmentedSplineParams_c5 C =
	{

		{ -4.0000000000, -4.0000000000, -3.1573765773, -0.4852499958, 1.8477324706, 1.8477324706 },

		{ -0.7185482425, 2.0810307172, 3.6681241237, 4.0000000000, 4.0000000000, 4.0000000000 },
		{ 0.18 * exp2(-15.0), 0.0001},
		{ 0.18, 4.8},
		{ 0.18 * exp2(18.0), 10000.},
		0.0,
		0.0
	};

	const int N_KNOTS_LOW = 4;
	const int N_KNOTS_HIGH = 4;

	const float KNOT_INC_LOW = (log10(C.midPoint.x) - log10(C.minPoint.x)) / (N_KNOTS_LOW - 1.);
	const float KNOT_INC_HIGH = (log10(C.maxPoint.x) - log10(C.midPoint.x)) / (N_KNOTS_HIGH - 1.);

	int i;


	float KNOT_Y_LOW[N_KNOTS_LOW];
	for (i = 0; i < N_KNOTS_LOW; i = i + 1)
	{
		KNOT_Y_LOW[i] = (C.coefsLow[i] + C.coefsLow[i + 1]) / 2.;
	};

	float KNOT_Y_HIGH[N_KNOTS_HIGH];
	for (i = 0; i < N_KNOTS_HIGH; i = i + 1)
	{
		KNOT_Y_HIGH[i] = (C.coefsHigh[i] + C.coefsHigh[i + 1]) / 2.;
	};

	float logy = log10(max(y, 1e-10));

	float logx;
	if (logy <= log10(C.minPoint.y))
	{
		logx = log10(C.minPoint.x);
	}
	else if ((logy > log10(C.minPoint.y)) && (logy <= log10(C.midPoint.y)))
	{
		uint j;
		float3 cf;
		if (logy > KNOT_Y_LOW[0] && logy <= KNOT_Y_LOW[1]) {
			cf[0] = C.coefsLow[0]; cf[1] = C.coefsLow[1]; cf[2] = C.coefsLow[2]; j = 0;
		}
		else if (logy > KNOT_Y_LOW[1] && logy <= KNOT_Y_LOW[2]) {
			cf[0] = C.coefsLow[1]; cf[1] = C.coefsLow[2]; cf[2] = C.coefsLow[3]; j = 1;
		}
		else if (logy > KNOT_Y_LOW[2] && logy <= KNOT_Y_LOW[3]) {
			cf[0] = C.coefsLow[2]; cf[1] = C.coefsLow[3]; cf[2] = C.coefsLow[4]; j = 2;
		}

		const float3 tmp = mul(cf, M);

		float a = tmp[0];
		float b = tmp[1];
		float c = tmp[2];
		c = c - logy;

		const float d = sqrt(b * b - 4. * a * c);

		const float t = (2. * c) / (-d - b);

		logx = log10(C.minPoint.x) + (t + j) * KNOT_INC_LOW;
	}
	else if ((logy > log10(C.midPoint.y)) && (logy < log10(C.maxPoint.y)))
	{
		uint j;
		float3 cf;
		if (logy > KNOT_Y_HIGH[0] && logy <= KNOT_Y_HIGH[1]) {
			cf[0] = C.coefsHigh[0]; cf[1] = C.coefsHigh[1]; cf[2] = C.coefsHigh[2]; j = 0;
		}
		else if (logy > KNOT_Y_HIGH[1] && logy <= KNOT_Y_HIGH[2]) {
			cf[0] = C.coefsHigh[1]; cf[1] = C.coefsHigh[2]; cf[2] = C.coefsHigh[3]; j = 1;
		}
		else if (logy > KNOT_Y_HIGH[2] && logy <= KNOT_Y_HIGH[3]) {
			cf[0] = C.coefsHigh[2]; cf[1] = C.coefsHigh[3]; cf[2] = C.coefsHigh[4]; j = 2;
		}

		const float3 tmp = mul(cf, M);

		float a = tmp[0];
		float b = tmp[1];
		float c = tmp[2];
		c = c - logy;

		const float d = sqrt(b * b - 4. * a * c);

		const float t = (2. * c) / (-d - b);

		logx = log10(C.midPoint.x) + (t + j) * KNOT_INC_HIGH;
	}
	else
	{
		logx = log10(C.maxPoint.x);
	}

	return pow(10, logx);
}

float segmented_spline_c9_fwd(float x, const SegmentedSplineParams_c9 C)
{
	const int N_KNOTS_LOW = 8;
	const int N_KNOTS_HIGH = 8;



	float xCheck = x <= 0 ? 1e-4 : x;

	float logx = log10(xCheck);
	float logy;

	if (logx <= log10(C.minPoint.x))
	{
		logy = logx * C.slopeLow + (log10(C.minPoint.y) - C.slopeLow * log10(C.minPoint.x));
	}
	else if ((logx > log10(C.minPoint.x)) && (logx < log10(C.midPoint.x)))
	{
		float knot_coord = (N_KNOTS_LOW - 1) * (logx - log10(C.minPoint.x)) / (log10(C.midPoint.x) - log10(C.minPoint.x));
		int j = knot_coord;
		float t = knot_coord - j;

		float3 cf = { C.coefsLow[j], C.coefsLow[j + 1], C.coefsLow[j + 2] };

		float3 monomials = { t * t, t, 1.0 };
		logy = dot(monomials, mul(cf, M));
	}
	else if ((logx >= log10(C.midPoint.x)) && (logx < log10(C.maxPoint.x)))
	{
		float knot_coord = (N_KNOTS_HIGH - 1) * (logx - log10(C.midPoint.x)) / (log10(C.maxPoint.x) - log10(C.midPoint.x));
		int j = knot_coord;
		float t = knot_coord - j;

		float3 cf = { C.coefsHigh[j], C.coefsHigh[j + 1], C.coefsHigh[j + 2] };

		float3 monomials = { t * t, t, 1.0 };
		logy = dot(monomials, mul(cf, M));
	}
	else
	{
		logy = logx * C.slopeHigh + (log10(C.maxPoint.y) - C.slopeHigh * log10(C.maxPoint.x));
	}

	return pow(10, logy);
}

float segmented_spline_c9_rev(float y, const SegmentedSplineParams_c9 C)
{
	const int N_KNOTS_LOW = 8;
	const int N_KNOTS_HIGH = 8;

	const float KNOT_INC_LOW = (log10(C.midPoint.x) - log10(C.minPoint.x)) / (N_KNOTS_LOW - 1.);
	const float KNOT_INC_HIGH = (log10(C.maxPoint.x) - log10(C.midPoint.x)) / (N_KNOTS_HIGH - 1.);

	int i;


	float KNOT_Y_LOW[N_KNOTS_LOW];
	for (i = 0; i < N_KNOTS_LOW; i = i + 1) {
		KNOT_Y_LOW[i] = (C.coefsLow[i] + C.coefsLow[i + 1]) / 2.;
	};

	float KNOT_Y_HIGH[N_KNOTS_HIGH];
	for (i = 0; i < N_KNOTS_HIGH; i = i + 1) {
		KNOT_Y_HIGH[i] = (C.coefsHigh[i] + C.coefsHigh[i + 1]) / 2.;
	};

	float logy = log10(max(y, 1e-10));

	float logx;
	if (logy <= log10(C.minPoint.y)) {
		logx = log10(C.minPoint.x);
	}
	else if ((logy > log10(C.minPoint.y)) && (logy <= log10(C.midPoint.y))) {
		uint j;
		float3 cf;
		if (logy > KNOT_Y_LOW[0] && logy <= KNOT_Y_LOW[1]) {
			cf[0] = C.coefsLow[0]; cf[1] = C.coefsLow[1]; cf[2] = C.coefsLow[2]; j = 0;
		}
		else if (logy > KNOT_Y_LOW[1] && logy <= KNOT_Y_LOW[2]) {
			cf[0] = C.coefsLow[1]; cf[1] = C.coefsLow[2]; cf[2] = C.coefsLow[3]; j = 1;
		}
		else if (logy > KNOT_Y_LOW[2] && logy <= KNOT_Y_LOW[3]) {
			cf[0] = C.coefsLow[2]; cf[1] = C.coefsLow[3]; cf[2] = C.coefsLow[4]; j = 2;
		}
		else if (logy > KNOT_Y_LOW[3] && logy <= KNOT_Y_LOW[4]) {
			cf[0] = C.coefsLow[3]; cf[1] = C.coefsLow[4]; cf[2] = C.coefsLow[5]; j = 3;
		}
		else if (logy > KNOT_Y_LOW[4] && logy <= KNOT_Y_LOW[5]) {
			cf[0] = C.coefsLow[4]; cf[1] = C.coefsLow[5]; cf[2] = C.coefsLow[6]; j = 4;
		}
		else if (logy > KNOT_Y_LOW[5] && logy <= KNOT_Y_LOW[6]) {
			cf[0] = C.coefsLow[5]; cf[1] = C.coefsLow[6]; cf[2] = C.coefsLow[7]; j = 5;
		}
		else if (logy > KNOT_Y_LOW[6] && logy <= KNOT_Y_LOW[7]) {
			cf[0] = C.coefsLow[6]; cf[1] = C.coefsLow[7]; cf[2] = C.coefsLow[8]; j = 6;
		}

		const float3 tmp = mul(cf, M);

		float a = tmp[0];
		float b = tmp[1];
		float c = tmp[2];
		c = c - logy;

		const float d = sqrt(b * b - 4. * a * c);

		const float t = (2. * c) / (-d - b);

		logx = log10(C.minPoint.x) + (t + j) * KNOT_INC_LOW;
	}
	else if ((logy > log10(C.midPoint.y)) && (logy < log10(C.maxPoint.y))) {
		uint j;
		float3 cf;
		if (logy > KNOT_Y_HIGH[0] && logy <= KNOT_Y_HIGH[1]) {
			cf[0] = C.coefsHigh[0]; cf[1] = C.coefsHigh[1]; cf[2] = C.coefsHigh[2]; j = 0;
		}
		else if (logy > KNOT_Y_HIGH[1] && logy <= KNOT_Y_HIGH[2]) {
			cf[0] = C.coefsHigh[1]; cf[1] = C.coefsHigh[2]; cf[2] = C.coefsHigh[3]; j = 1;
		}
		else if (logy > KNOT_Y_HIGH[2] && logy <= KNOT_Y_HIGH[3]) {
			cf[0] = C.coefsHigh[2]; cf[1] = C.coefsHigh[3]; cf[2] = C.coefsHigh[4]; j = 2;
		}
		else if (logy > KNOT_Y_HIGH[3] && logy <= KNOT_Y_HIGH[4]) {
			cf[0] = C.coefsHigh[3]; cf[1] = C.coefsHigh[4]; cf[2] = C.coefsHigh[5]; j = 3;
		}
		else if (logy > KNOT_Y_HIGH[4] && logy <= KNOT_Y_HIGH[5]) {
			cf[0] = C.coefsHigh[4]; cf[1] = C.coefsHigh[5]; cf[2] = C.coefsHigh[6]; j = 4;
		}
		else if (logy > KNOT_Y_HIGH[5] && logy <= KNOT_Y_HIGH[6]) {
			cf[0] = C.coefsHigh[5]; cf[1] = C.coefsHigh[6]; cf[2] = C.coefsHigh[7]; j = 5;
		}
		else if (logy > KNOT_Y_HIGH[6] && logy <= KNOT_Y_HIGH[7]) {
			cf[0] = C.coefsHigh[6]; cf[1] = C.coefsHigh[7]; cf[2] = C.coefsHigh[8]; j = 6;
		}

		const float3 tmp = mul(cf, M);

		float a = tmp[0];
		float b = tmp[1];
		float c = tmp[2];
		c = c - logy;

		const float d = sqrt(b * b - 4. * a * c);

		const float t = (2. * c) / (-d - b);

		logx = log10(C.midPoint.x) + (t + j) * KNOT_INC_HIGH;
	}
	else
	{
		logx = log10(C.maxPoint.x);
	}

	return pow(10, logx);
}


float rgb_2_hue(float3 rgb)
{


	float hue;
	if (rgb[0] == rgb[1] && rgb[1] == rgb[2])
	{

		hue = 0;
	}
	else
	{
		hue = (180. / PI) * atan2(sqrt(3.0) * (rgb[1] - rgb[2]), 2 * rgb[0] - rgb[1] - rgb[2]);
	}

	if (hue < 0.)
		hue = hue + 360;

	return clamp(hue, 0, 360);
}

float rgb_2_yc(float3 rgb, float ycRadiusWeight = 1.75)
{













	float r = rgb[0];
	float g = rgb[1];
	float b = rgb[2];

	float chroma = sqrt(b * (b - g) + g * (g - r) + r * (r - b));

	return (b + g + r + ycRadiusWeight * chroma) / 3.;
}







float3 RRT(float3 aces)
{

	const float RRT_GLOW_GAIN = 0.05;
	const float RRT_GLOW_MID = 0.08;

	float saturation = rgb_2_saturation(aces);
	float ycIn = rgb_2_yc(aces);
	float s = sigmoid_shaper((saturation - 0.4) / 0.2);
	float addedGlow = 1 + glow_fwd(ycIn, RRT_GLOW_GAIN * s, RRT_GLOW_MID);
	aces *= addedGlow;


	const float RRT_RED_SCALE = 0.82;
	const float RRT_RED_PIVOT = 0.03;
	const float RRT_RED_HUE = 0;
	const float RRT_RED_WIDTH = 135;
	float hue = rgb_2_hue(aces);
	float centeredHue = center_hue(hue, RRT_RED_HUE);
	float hueWeight = cubic_basis_shaper(centeredHue, RRT_RED_WIDTH);

	aces.r += hueWeight * saturation * (RRT_RED_PIVOT - aces.r) * (1. - RRT_RED_SCALE);


	aces = clamp(aces, 0, 65535);

	float3 rgbPre = mul(AP0_2_AP1_MAT, aces);

	rgbPre = clamp(rgbPre, 0, 65535);


	const float RRT_SAT_FACTOR = 0.96;
	rgbPre = lerp(dot(rgbPre, AP1_RGB2Y), rgbPre, RRT_SAT_FACTOR);


	float3 rgbPost;
	rgbPost[0] = segmented_spline_c5_fwd(rgbPre[0]);
	rgbPost[1] = segmented_spline_c5_fwd(rgbPre[1]);
	rgbPost[2] = segmented_spline_c5_fwd(rgbPre[2]);


	float3 rgbOces = mul(AP1_2_AP0_MAT, rgbPost);
	return rgbOces;
}








float3 Inverse_RRT(float3 oces)
{

	const float RRT_GLOW_GAIN = 0.05;
	const float RRT_GLOW_MID = 0.08;


	float3 rgbPre = mul(AP0_2_AP1_MAT, oces);


	float3 rgbPost;
	rgbPost[0] = segmented_spline_c5_rev(rgbPre[0]);
	rgbPost[1] = segmented_spline_c5_rev(rgbPre[1]);
	rgbPost[2] = segmented_spline_c5_rev(rgbPre[2]);


	const float RRT_SAT_FACTOR = 0.96;
	rgbPost = lerp(dot(rgbPost, AP1_RGB2Y), rgbPost, rcp(RRT_SAT_FACTOR));

	rgbPost = clamp(rgbPost, 0., HALF_MAX);


	float3 aces = mul(AP1_2_AP0_MAT, rgbPost);

	aces = clamp(aces, 0., HALF_MAX);


	const float RRT_RED_SCALE = 0.82;
	const float RRT_RED_PIVOT = 0.03;
	const float RRT_RED_HUE = 0;
	const float RRT_RED_WIDTH = 135;
	float hue = rgb_2_hue(aces);
	float centeredHue = center_hue(hue, RRT_RED_HUE);
	float hueWeight = cubic_basis_shaper(centeredHue, RRT_RED_WIDTH);

	float minChan;
	if (centeredHue < 0) {

		minChan = aces[1];
	}
	else {
		minChan = aces[2];
	}

	float a = hueWeight * (1. - RRT_RED_SCALE) - 1.;
	float b = aces[0] - hueWeight * (RRT_RED_PIVOT + minChan) * (1. - RRT_RED_SCALE);
	float c = hueWeight * RRT_RED_PIVOT * minChan * (1. - RRT_RED_SCALE);

	aces[0] = (-b - sqrt(b * b - 4. * a * c)) / (2. * a);


	float saturation = rgb_2_saturation(aces);
	float ycOut = rgb_2_yc(aces);
	float s = sigmoid_shaper((saturation - 0.4) / 0.2);
	float reducedGlow = 1. + glow_inv(ycOut, RRT_GLOW_GAIN * s, RRT_GLOW_MID);

	aces = reducedGlow * aces;


	return aces;
}





float3 XYZ_2_xyY(float3 XYZ)
{
	float3 xyY;
	float divisor = (XYZ[0] + XYZ[1] + XYZ[2]);
	if (divisor == 0.) divisor = 1e-10;
	xyY[0] = XYZ[0] / divisor;
	xyY[1] = XYZ[1] / divisor;
	xyY[2] = XYZ[1];

	return xyY;
}

float3 xyY_2_XYZ(float3 xyY)
{
	float3 XYZ;
	XYZ[0] = xyY[0] * xyY[2] / max(xyY[1], 1e-10);
	XYZ[1] = xyY[2];
	XYZ[2] = (1.0 - xyY[0] - xyY[1]) * xyY[2] / max(xyY[1], 1e-10);

	return XYZ;
}


float3x3 ChromaticAdaptation(float2 src_xy, float2 dst_xy)
{



	const float3x3 ConeResponse =
	{
		 0.8951, 0.2664, -0.1614,
		-0.7502, 1.7135, 0.0367,
		 0.0389, -0.0685, 1.0296,
	};
	const float3x3 InvConeResponse =
	{
		 0.9869929, -0.1470543, 0.1599627,
		 0.4323053, 0.5183603, 0.0492912,
		-0.0085287, 0.0400428, 0.9684867,
	};

	float3 src_XYZ = xyY_2_XYZ(float3(src_xy, 1));
	float3 dst_XYZ = xyY_2_XYZ(float3(dst_xy, 1));

	float3 src_coneResp = mul(ConeResponse, src_XYZ);
	float3 dst_coneResp = mul(ConeResponse, dst_XYZ);

	float3x3 VonKriesMat =
	{
		{ dst_coneResp[0] / src_coneResp[0], 0.0, 0.0 },
		{ 0.0, dst_coneResp[1] / src_coneResp[1], 0.0 },
		{ 0.0, 0.0, dst_coneResp[2] / src_coneResp[2] }
	};

	return mul(InvConeResponse, mul(VonKriesMat, ConeResponse));
}

float Y_2_linCV(float Y, float Ymax, float Ymin)
{
	return (Y - Ymin) / (Ymax - Ymin);
}

float linCV_2_Y(float linCV, float Ymax, float Ymin)
{
	return linCV * (Ymax - Ymin) + Ymin;
}


static const float DIM_SURROUND_GAMMA = 0.9811;

float3 darkSurround_to_dimSurround(float3 linearCV)
{
	float3 XYZ = mul(AP1_2_XYZ_MAT, linearCV);

	float3 xyY = XYZ_2_xyY(XYZ);
	xyY[2] = clamp(xyY[2], 0, 65535);
	xyY[2] = pow(xyY[2], DIM_SURROUND_GAMMA);
	XYZ = xyY_2_XYZ(xyY);

	return mul(XYZ_2_AP1_MAT, XYZ);
}

float3 dimSurround_to_darkSurround(float3 linearCV)
{
	float3 XYZ = mul(linearCV, AP1_2_XYZ_MAT);

	float3 xyY = XYZ_2_xyY(XYZ);
	xyY[2] = clamp(xyY[2], 0., 65535);
	xyY[2] = pow(xyY[2], 1. / DIM_SURROUND_GAMMA);
	XYZ = xyY_2_XYZ(xyY);

	return mul(XYZ, XYZ_2_AP1_MAT);
}

















































float3 ODT_sRGB_D65(float3 oces)
{

	float3 rgbPre = mul(AP0_2_AP1_MAT, oces);

	const SegmentedSplineParams_c9 ODT_48nits =
	{

		{ -1.6989700043, -1.6989700043, -1.4779000000, -1.2291000000, -0.8648000000, -0.4480000000, 0.0051800000, 0.4511080334, 0.9113744414, 0.9113744414},

		{ 0.5154386965, 0.8470437783, 1.1358000000, 1.3802000000, 1.5197000000, 1.5985000000, 1.6467000000, 1.6746091357, 1.6878733390, 1.6878733390 },
		{segmented_spline_c5_fwd(0.18 * exp2(-6.5)), 0.02},
		{segmented_spline_c5_fwd(0.18), 4.8},
		{segmented_spline_c5_fwd(0.18 * exp2(6.5)), 48.0},
		0.0,
		0.04
	};


	float3 rgbPost;
	rgbPost[0] = segmented_spline_c9_fwd(rgbPre[0], ODT_48nits);
	rgbPost[1] = segmented_spline_c9_fwd(rgbPre[1], ODT_48nits);
	rgbPost[2] = segmented_spline_c9_fwd(rgbPre[2], ODT_48nits);


	const float CINEMA_WHITE = 48.0;
	const float CINEMA_BLACK = 0.02;


	float3 linearCV;
	linearCV[0] = Y_2_linCV(rgbPost[0], CINEMA_WHITE, CINEMA_BLACK);
	linearCV[1] = Y_2_linCV(rgbPost[1], CINEMA_WHITE, CINEMA_BLACK);
	linearCV[2] = Y_2_linCV(rgbPost[2], CINEMA_WHITE, CINEMA_BLACK);


	linearCV = darkSurround_to_dimSurround(linearCV);


	const float ODT_SAT_FACTOR = 0.93;
	linearCV = lerp(dot(linearCV, AP1_RGB2Y), linearCV, ODT_SAT_FACTOR);



	float3 XYZ = mul(AP1_2_XYZ_MAT, linearCV);


	XYZ = mul(D60_2_D65_CAT, XYZ);


	linearCV = mul(XYZ_2_sRGB_MAT, XYZ);



	linearCV = saturate(linearCV);

	return linearCV;
}





float3 Inverse_ODT_sRGB_D65(float3 linearCV)
{


	float3 XYZ = mul(sRGB_2_XYZ_MAT, linearCV);


	linearCV = mul(XYZ_2_AP1_MAT, XYZ);


	XYZ = mul(D65_2_D60_CAT, XYZ);


	const float ODT_SAT_FACTOR = 0.93;
	linearCV = lerp(dot(linearCV, AP1_RGB2Y), linearCV, rcp(ODT_SAT_FACTOR));


	linearCV = dimSurround_to_darkSurround(linearCV);


	const float CINEMA_WHITE = 48.0;
	const float CINEMA_BLACK = 0.02;


	float3 rgbPre;
	rgbPre[0] = linCV_2_Y(linearCV[0], CINEMA_WHITE, CINEMA_BLACK);
	rgbPre[1] = linCV_2_Y(linearCV[1], CINEMA_WHITE, CINEMA_BLACK);
	rgbPre[2] = linCV_2_Y(linearCV[2], CINEMA_WHITE, CINEMA_BLACK);

	const SegmentedSplineParams_c9 ODT_48nits =
	{

		{ -1.6989700043, -1.6989700043, -1.4779000000, -1.2291000000, -0.8648000000, -0.4480000000, 0.0051800000, 0.4511080334, 0.9113744414, 0.9113744414},

		{ 0.5154386965, 0.8470437783, 1.1358000000, 1.3802000000, 1.5197000000, 1.5985000000, 1.6467000000, 1.6746091357, 1.6878733390, 1.6878733390 },
		{segmented_spline_c5_fwd(0.18 * pow(2.,-6.5)), 0.02},
		{segmented_spline_c5_fwd(0.18), 4.8},
		{segmented_spline_c5_fwd(0.18 * pow(2.,6.5)), 48.0},
		0.0,
		0.04
	};


	float3 rgbPost;
	rgbPost[0] = segmented_spline_c9_rev(rgbPre[0], ODT_48nits);
	rgbPost[1] = segmented_spline_c9_rev(rgbPre[1], ODT_48nits);
	rgbPost[2] = segmented_spline_c9_rev(rgbPre[2], ODT_48nits);


	float3 oces = mul(AP1_2_AP0_MAT, rgbPost);

	return oces;
}


































float3 ODT_1000nits(float3 oces)
{

	float3 rgbPre = mul(AP0_2_AP1_MAT, oces);

	const SegmentedSplineParams_c9 ODT_1000nits =
	{

		{ -4.9706219331, -3.0293780669, -2.1262, -1.5105, -1.0578, -0.4668, 0.11938, 0.7088134201, 1.2911865799, 1.2911865799 },

		{ 0.8089132070, 1.1910867930, 1.5683, 1.9483, 2.3083, 2.6384, 2.8595, 2.9872608805, 3.0127391195, 3.0127391195 },
		{segmented_spline_c5_fwd(0.18 * pow(2.,-12.)), 0.0001},
		{segmented_spline_c5_fwd(0.18), 10.0},
		{segmented_spline_c5_fwd(0.18 * pow(2.,10.)), 1000.0},
		3.0,
		0.06
	};


	float3 rgbPost;
	rgbPost[0] = segmented_spline_c9_fwd(rgbPre[0], ODT_1000nits);
	rgbPost[1] = segmented_spline_c9_fwd(rgbPre[1], ODT_1000nits);
	rgbPost[2] = segmented_spline_c9_fwd(rgbPre[2], ODT_1000nits);


	rgbPost -= 0.00003507384284432574;

	return rgbPost;
}



































float3 ODT_2000nits(float3 oces)
{

	float3 rgbPre = mul(AP0_2_AP1_MAT, oces);

	const SegmentedSplineParams_c9 ODT_2000nits =
	{

	  { -2.3010299957, -2.3010299957, -1.9312000000, -1.5205000000, -1.0578000000, -0.4668000000, 0.1193800000, 0.7088134201, 1.2911865799, 1.2911865799 },

	  { 0.8019952042, 1.1980047958, 1.5943000000, 1.9973000000, 2.3783000000, 2.7684000000, 3.0515000000, 3.2746293562, 3.3274306351, 3.3274306351 },
	  {segmented_spline_c5_fwd(0.18 * pow(2.,-12.)), 0.005},
	  {segmented_spline_c5_fwd(0.18), 10.0},
	  {segmented_spline_c5_fwd(0.18 * pow(2.,11.)), 2000.0},
	  0.0,
	  0.12
	};


	float3 rgbPost;
	rgbPost[0] = segmented_spline_c9_fwd(rgbPre[0], ODT_2000nits);
	rgbPost[1] = segmented_spline_c9_fwd(rgbPre[1], ODT_2000nits);
	rgbPost[2] = segmented_spline_c9_fwd(rgbPre[2], ODT_2000nits);

	return rgbPost;
}