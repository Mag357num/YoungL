struct Material
{
	float Shiness;
	float3 Fresnel0;
	float4 DiffuseAlbedo;
	float3 AmbientLight;
};

struct DirectionLight
{
	float3 Strength;
	float3 Position;
	float3 Direction;
};

//PS color = Blinnphonecolor + AmbientColor(AmbientLight*DiffuseColor)

//blinn phone

//color = (diffuse + specular)*lightStrength;;diffuse:from material data diffusealbedo
//specular = Fresnellfactor* roughnessfactor

//fresnellfactor from schlick formula: F = R0 +(1-Ro)*(F0)^4;
//R0: Fresnel0 From material data
//F0: 1-dot(halfvec , lightVec)
//halfvec = (lightVec + viewdirection)

//roughtness factor from micro-face model (m+8)*(dot(halfvec, normal)^m)/8
//m:shiness*256.0

float3 ComputeBlinnPhone_DirectionalLight(DirectionLight Light, Material Mat, float3 Normal, float3 ViewDirection)
{
	//oppsite light direction
	float3 LightVec = -Light.Direction;

	float3 AmbientColor = Mat.DiffuseAlbedo.rgb * Mat.AmbientLight;

	//calc half vec
	//opposite of light direction
	float3 HalfVec = ViewDirection + LightVec;
	HalfVec = normalize(HalfVec);

	float OneMinusFo = dot(HalfVec, LightVec);
	OneMinusFo = saturate(OneMinusFo);
	float F0 = 1.0f - OneMinusFo;
	float3 FresnelFactor = Mat.Fresnel0 + (1.0 - Mat.Fresnel0) * F0 * F0 * F0 * F0;

	float m = Mat.Shiness * 256.0f;
	float HalfDotNormal = dot(HalfVec, Normal);
	HalfDotNormal = max(HalfDotNormal, 0.0f);
	float PowerM = pow(HalfDotNormal, m);
	float RoughnessFactor = (m + 8.0f) * PowerM / 8.0f;
	float3 SpecularAelbedo = FresnelFactor * RoughnessFactor;

	//aovid out of 1
	SpecularAelbedo = SpecularAelbedo / (SpecularAelbedo + 1.0f);

	float3 BlinnPhoneColor = (Mat.DiffuseAlbedo.rgb + SpecularAelbedo)* Light.Strength;
	
	return BlinnPhoneColor + AmbientColor;
}