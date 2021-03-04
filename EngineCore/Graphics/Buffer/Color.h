#pragma once
#include <DirectXMath.h>

using namespace DirectX;

class Color
{
public: 
	
	Color():m_Value(g_XMOne){}
	Color(FXMVECTOR Vec);
	Color(const XMVECTORF32& Vec);
	Color(float R, float G, float B, float A = 1.0f);
	Color(uint16_t R, uint16_t G, uint16_t B, uint16_t A = 255, uint16_t BitDepth = 8);
	explicit Color(uint32_t RGBALittleEndian);

	float R() const { return XMVectorGetX(m_Value); }
	float G() const { return XMVectorGetY(m_Value); }
	float B() const { return XMVectorGetZ(m_Value); }
	float A() const { return XMVectorGetW(m_Value); }

	bool operator==(const Color& Rhs) const { return XMVector4Equal(m_Value, Rhs.m_Value); }
	bool operator!=(const Color& Rhs) const { return !XMVector4Equal(m_Value, Rhs.m_Value); }

	void SetR(float R) { m_Value.f[0] = R; }
	void SetG(float G) { m_Value.f[1] = G; }
	void SetB(float B) { m_Value.f[2] = B; }
	void SetA(float A) { m_Value.f[3] = A; }

	float* GetPtr(void) { return reinterpret_cast<float*>(this); }
	float& operator[](int Idx) { return GetPtr()[Idx]; }

	void SetRGB(float R, float G, float B) { m_Value.v = XMVectorSelect(m_Value, XMVectorSet(R, G, B, B), g_XMMask3); }

	Color ToSRGB() const;
	Color FromSRGB() const;
	Color ToRec709() const;
	Color FromRec709() const;

	//probably want to convert to srgb or rec709 first
	uint32_t R10G10B10A2() const;
	uint32_t R8G8B8A8() const;

	//pak an hdr color into 32-bits
	uint32_t R11G11B10F(bool RoundToEven = false) const;
	uint32_t R9G9B9E5() const;

	operator XMVECTOR() const { return m_Value; }

private:
	XMVECTORF32 m_Value;
};

INLINE Color Max(Color A, Color B) { return Color(XMVectorMax(A, B)); }
INLINE Color Min(Color A, Color B) { return Color(XMVectorMin(A, B)); }
INLINE Color Clamp(Color X, Color A, Color B) { return Color(XMVectorClamp(X, A, B)); }

inline Color::Color(FXMVECTOR Vec)
{
	m_Value.v = Vec;
}

inline Color::Color(const FXMVECTORF32& Vec)
{
	m_Value.v = Vec;
}

inline Color::Color(float R, float G, float B, float A)
{
	m_Value.v = XMVectorSet(R, G, B, A);
}

inline Color::Color(uint16_t R, uint16_t G, uint16_t B, uint16_t A, uint16_t BitDepth)
{
	m_Value.v = XMVectorScale(XMVectorSet(R, G, B, A), 1.0f / ((1<<BitDepth) - 1));
}

inline Color::Color(uint32_t u32)
{
	float R = (float)((u32 >> 0) & 0xFF);
	float G = (float)((u32 >> 8) & 0xFF);
	float B = (float)((u32 >> 16) & 0xFF);
	float A = (float)((u32 >> 24) & 0xFF);
	m_Value.v = XMVectorScale(XMVectorSet(R, G, B, A), 1.0f / 255.0f);
}

//linear color to srgb
//s = 1.055 * Math.pow(l, 1.0/2.4) - 0.055
inline Color Color::ToSRGB(void) const
{
	XMVECTOR T = XMVectorSaturate(m_Value);
	XMVECTOR Result = XMVectorSubtract(XMVectorScale(XMVectorPow(T, XMVectorReplicate(1.0f / 2.4f)), 1.055f), XMVectorReplicate(0.055f));
	Result = XMVectorSelect(Result, XMVectorScale(T, 12.92f), XMVectorLess(T, XMVectorReplicate(0.0031308f)));
	return XMVectorSelect(T, Result, g_XMSelect1110);
}

inline Color Color::FromSRGB(void) const
{
	XMVECTOR T XMVectorSaturate(m_Value);
	XMVECTOR Result = XMVectorPow(XMVectorScale(XMVectorAdd(T, T, XMVectorReplicate(0.055f)), 1.0f / 1.055f), XMVectorReplicate(2.4f));
	Result = XMVectorSelect(Result, XMVectorScale(T, 1.0f / 12.92f), XMVectorLess(T, XMVectorReplicate(0.0031308f)));
	return XMVectorSelect(T, Result, g_XMSelect1110);
}

inline Color Color::ToRec709(void) const
{
	XMVECTOR T = XMVectorSaturate(m_Value);
	XMVECTOR Result = XMVectorSubtract(XMVectorScale(XMVectorPow(T, XMVectorReplicate(0.45f)), 1.099f), XMVectorReplicate(0.099f));
	Result = XMVectorSelect(Result, XMVectorScale(T, 4.5f), XMVectorLess(T, XMVectorReplicate(.00018f)));
	return XMVectorSelect(T, Result, g_XMSelect1110);
}

inline Color Color::FromRec709(void) const
{
	XMVECTOR T = XMVectorSaturate(m_Value);
	XMVECTOR Result = XMVectorPow(XMVectorScale(XMVectorAdd(T, XMVectorReplicate(0.099f)), 1.0f / 1.099f), XMVectorReplicate(1.0f / 0.45f));
	Result = XMVectorSelect(Result, XMVectorScale(T, 1.0f / 4.5f), XMVectorLess(T, XMVectorReplicate(0.0081f)));
	return XMVectorSelect(T, Result, g_XMSelect1110);
}

inline uint32_t Color::R10G10B10A2(void) const
{
	XMVECTOR Result = XMVectorRound(XMVectorMultiply(XMVectorSaturate(m_Value), XMVectorSet(1023.0f, 1023.0f, 1023.0f, 3.0f)));
	Result = _mm_castsi128_ps(_mm_cvttps_epi32(Result));
	uint32_t R = XMVectorGetIntX(Result);
	uint32_t G = XMVectorGetIntY(Result);
	uint32_t B = XMVectorGetIntZ(Result);
	uint32_t A = XMVectorGetIntW(Result) >> 8;

	return A << 30 | B << 20 | G << 10 | R;
}



inline uint32_t Color::R8G8B8A8(void) const
{
	XMVECTOR result = XMVectorRound(XMVectorMultiply(XMVectorSaturate(m_value), XMVectorReplicate(255.0f)));
	result = _mm_castsi128_ps(_mm_cvttps_epi32(result));
	uint32_t r = XMVectorGetIntX(result);
	uint32_t g = XMVectorGetIntY(result);
	uint32_t b = XMVectorGetIntZ(result);
	uint32_t a = XMVectorGetIntW(result);
	return a << 24 | b << 16 | g << 8 | r;
}