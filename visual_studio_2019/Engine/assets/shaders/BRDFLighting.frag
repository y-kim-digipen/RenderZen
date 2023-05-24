#version 450

#define PI 3.14159265359
#define INV_PI 1.0 / PI

#define MAX_SCENE_POINT_LIGHT 2
#define MAX_SCENE_SPHERE_LIGHT 2

struct MicrofacetModel
{
    vec3 albedo;
    float metallic;
    vec3 N; //microfacet model normal
    float roughness;
    vec3 V; //view vector
    float emission;
    vec3 L; //light vector
    float padding;
    vec3 pos;
};

struct BRDFPrecomputedInput
{
    vec3 H;
    float NdotV;
    float LdotH;
    float NdotH;
    float NdotL;
    float sqaureRoughness;
};

//Punctual light
struct PointLight
{
	vec3 position;
	float lumen;
	vec3 RGBColor;
	float radius;
};

//Area light
struct SphereLight
{
	vec3 position;
	float lumen;
	vec3 RGBColor;
	float radius;
};

//Reference
#define DEFAULT_REFLECTANCE 0.04
#define DEFAULT_F90 vec3(1)

// ------------------------------ ?? ------------------------------

float SchlickGGX_D(float NoH, float m);
float SchlickGGX_D(vec3 norm, vec3 h, float roughness);
float SchlickGGX_G1(vec3 norm, vec3 w, float roughness);
float SchlickGGX_SmithG(vec3 norm, vec3 wo, vec3 wi, float roughness);
// Disney : reduce "hotness" for analytic light sources
float SchlickGGX_SmithG_Analytic(vec3 norm, vec3 wo, vec3 wi, float roughness);
// G / 4(NoV)(NoL), height correlated
float SchlickGGX_SmithG_Vis_HC(float NoV, float NoL, float alphaG);
vec3 SchlickGGX_Sample(vec2 Xi, vec3 norm, float roughness);

vec3 MetalWorkflowF0(vec3 albedo, float metallic);
vec3 MetalWorkflowF0(vec3 reflectance, vec3 albedo, float metallic);
float SchlickFr(float F0, float F90, float u);
vec3 SchlickFr(vec3 F0, vec3 F90, float u);
vec3 SchlickFr(vec3 F0, float u);
vec3 SchlickFr(float u, vec3 albedo, float metallic);
vec3 SchlickFr(vec3 w, vec3 h, vec3 albedo, float metallic);
vec3 SchlickFrR(vec3 wo, vec3 norm, vec3 F0, float roughness);
float DisneyDiffuseFr(float NoV, float NoL, float LoH, float linearRoughness);
float DisneyDiffuseFr(vec3 norm, vec3 wo, vec3 wi, float linearRoughness);

// ------------------------------ ?? ------------------------------

//float SchlickGGX_D(float NoH, float m) {
//	float m2 = m * m;
//	float NoH2 = NoH * NoH;
//	float f = (m2 - 1) * NoH2 + 1;
//	return m2 / (f * f);
//}
//
//float SchlickGGX_D(vec3 norm, vec3 h, float roughness){
//	float NoH = clamp(dot(norm, h), 0, 1);
//	
//	float alpha = roughness * roughness;
//	
//	float alpha2 = alpha * alpha;
//	float cos2Theta = NoH * NoH;
//	
//	float t = (alpha2 - 1) * cos2Theta + 1;
//	
//	return alpha2 / (PI * t * t);
//}
//
//float SchlickGGX_G1(vec3 norm, vec3 w, float roughness) {
//	float alpha = roughness * roughness;
//	float k = alpha / 2; // fix smith model ggx
//	
//	float NoW = max(0, dot(norm, w));
//	return NoW / (NoW * (1 - k) + k);
//}
//
//float SchlickGGX_SmithG(vec3 norm, vec3 wo, vec3 wi, float roughness){
//	return SchlickGGX_G1(norm, wo, roughness) * SchlickGGX_G1(norm, wi, roughness);
//}
//
//float SchlickGGX_SmithG_Analytic(vec3 norm, vec3 wo, vec3 wi, float roughness) {
//	float remappedRoughness = (roughness + 1) * 0.5;
//	return SchlickGGX_SmithG(norm, wo, wi, remappedRoughness);
//}
//
//float SchlickGGX_SmithG_Vis_HC(float NoV, float NoL, float alphaG) {
//	// Original formulation of G_SmithGGX Correlated
//	// lambda_v = (-1 + sqrt ( alphaG2 * (1 - NoL2 ) / NoL2 + 1)) * 0.5 f;
//	// lambda_l = (-1 + sqrt ( alphaG2 * (1 - NoV2 ) / NoV2 + 1)) * 0.5 f;
//	// G_SmithGGXCorrelated = 1 / (1 + lambda_v + lambda_l );
//	// V_SmithGGXCorrelated = G_SmithGGXCorrelated / (4.0 f * NoL * NoV );
//
//	// This is the optimize version
//	float alphaG2 = alphaG * alphaG;
//	// Caution : the " NoL *" and " NoV *" are explicitely inversed , this is not a mistake .
//	float Lambda_GGXV = NoL * sqrt((-NoV * alphaG2 + NoV) * NoV + alphaG2);
//	float Lambda_GGXL = NoV * sqrt((-NoL * alphaG2 + NoL) * NoL + alphaG2);
//
//	return 0.5 / (Lambda_GGXV + Lambda_GGXL);
//}
//
//vec3 MetalWorkflowF0(vec3 reflectance, vec3 albedo, float metallic) {
//	return mix(reflectance, albedo, metallic);
//}
//
//vec3 MetalWorkflowF0(vec3 albedo, float metallic){
//	return MetalWorkflowF0(vec3(DEFAULT_REFLECTANCE), albedo, metallic);
//}
//
//float SchlickFr(float F0, float F90, float u) {
//	float x = 1 - u;
//	float x2 = x * x;
//	float x4 = x2 * x2;
//	float x5 = x4 * x;
//	return F0 + (F90 - F0) * x5;
//}
//
//vec3 SchlickFr(vec3 F0, vec3 F90, float u) {
//	float x = 1 - u;
//	float x2 = x * x;
//	float x4 = x2 * x2;
//	float x5 = x4 * x;
//	return F0 + (F90 - F0) * x5;
//}
//
//vec3 SchlickFr(vec3 F0, float u) {
//	return SchlickFr(F0, DEFAULT_F90, u);
//}
//
//vec3 SchlickFr(float u, vec3 albedo, float metallic) {
//	vec3 F0 = MetalWorkflowF0(albedo, metallic);
//	return SchlickFr(F0, u);
//}
//
//vec3 SchlickFr(vec3 w, vec3 h, vec3 albedo, float metallic) {
//	vec3 F0 = MetalWorkflowF0(albedo, metallic);
//	float HoW = dot(h, w);
//	return F0 + exp2((-5.55473f * HoW - 6.98316f) * HoW) * (DEFAULT_F90 - F0);
//}
//
//vec3 SchlickFrR(vec3 wo, vec3 norm, vec3 F0, float roughness) {
//	float cosTheta = max(dot(wo, norm), 0);
//    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
//}
//
//float DisneyDiffuseFr(float NoV, float NoL, float LoH, float linearRoughness) {
//	float energyBias = mix(0.f, 0.5f, linearRoughness);
//	float energyFactor = mix(1.f, 1.f / 1.51f, linearRoughness);
//	float f90 = energyBias + 2.f * LoH * LoH * linearRoughness;
//	const float f0 = 1.f;
//	float lightScatter = SchlickFr(f0, f90, NoL);
//	float viewScatter = SchlickFr(f0, f90, NoV);
//
//	return lightScatter * viewScatter * energyFactor;
//}
//
//float DisneyDiffuseFr(vec3 norm, vec3 wo, vec3 wi, float linearRoughness) {
//	vec3 h = normalize(wo + wi);
//	float HoWi = dot(h, wi);
//	float HoWi2 = HoWi * HoWi;
//	
//	float NoWo = dot(norm, wo);
//	float NoWi = dot(norm, wi);
//
//	return DisneyDiffuseFr(NoWo, NoWi, HoWi, linearRoughness);
//}

//Ref end

///////////BRDF_private///////////
//Frostbite BRDF model "Moving Frostbite to PBR"

vec3 MetalWorkflowF0(vec3 reflectance, vec3 albedo, float metallic) {
	return mix(reflectance, albedo, metallic);
}

vec3 MetalWorkflowF0(vec3 albedo, float metallic){
	return MetalWorkflowF0(vec3(0.04), albedo, metallic);
}

float SchlickFr(float F0, float F90, float u) {
	float x = 1 - u;
	float x2 = x * x;
	float x4 = x2 * x2;
	float x5 = x4 * x;
	return F0 + (F90 - F0) * x5;
}

vec3 SchlickFr(vec3 F0, vec3 F90, float u) {
	float x = 1 - u;
	float x2 = x * x;
	float x4 = x2 * x2;
	float x5 = x4 * x;
	return F0 + (F90 - F0) * x5;
}

vec3 SchlickFr(vec3 F0, float u) {
	return SchlickFr(F0, vec3(1), u);
}

vec3 SchlickFr(float u, vec3 albedo, float metallic) {
	vec3 F0 = MetalWorkflowF0(albedo, metallic);
	return SchlickFr(F0, u);
}

float DisneyDiffuseFr(float NoV, float NoL, float LoH, float linearRoughness) {
	float energyBias = mix(0.f, 0.5f, linearRoughness);
	float energyFactor = mix(1.f, 1.f / 1.51f, linearRoughness);
	float f90 = energyBias + 2.f * LoH * LoH * linearRoughness;
	const float f0 = 1.f;
	float lightScatter = SchlickFr(f0, f90, NoL);
	float viewScatter = SchlickFr(f0, f90, NoV);

	return lightScatter * viewScatter * energyFactor;
}

float SchlickGGX_SmithG_Vis_HC(float NoV, float NoL, float alphaG) {
	// Original formulation of G_SmithGGX Correlated
	// lambda_v = (-1 + sqrt ( alphaG2 * (1 - NoL2 ) / NoL2 + 1)) * 0.5 f;
	// lambda_l = (-1 + sqrt ( alphaG2 * (1 - NoV2 ) / NoV2 + 1)) * 0.5 f;
	// G_SmithGGXCorrelated = 1 / (1 + lambda_v + lambda_l );
	// V_SmithGGXCorrelated = G_SmithGGXCorrelated / (4.0 f * NoL * NoV );

	// This is the optimize version
	float alphaG2 = alphaG * alphaG;
	// Caution : the " NoL *" and " NoV *" are explicitely inversed , this is not a mistake .
	float Lambda_GGXV = NoL * sqrt((-NoV * alphaG2 + NoV) * NoV + alphaG2);
	float Lambda_GGXL = NoV * sqrt((-NoL * alphaG2 + NoL) * NoL + alphaG2);

	return 0.5 / (Lambda_GGXV + Lambda_GGXL);
}

float SchlickGGX_D(float NoH, float m) {
	float m2 = m * m;
	float NoH2 = NoH * NoH;
	float f = (m2 - 1) * NoH2 + 1;
	return m2 / (f * f);
}

void ComputeBRDFInput(out BRDFPrecomputedInput brdfInput, MicrofacetModel model)
{
    vec3 V = model.V;
    vec3 N = model.N;
    vec3 L = model.L;

    vec3 H = normalize(V + L);

    brdfInput.H = H;
    brdfInput.NdotV = abs(dot(N, V)) + 0.00001;
    brdfInput.LdotH = clamp(dot(L, H), 0.0, 1.0);
    brdfInput.NdotH = clamp(dot(N, H), 0.0, 1.0);
    brdfInput.NdotL = clamp(dot(N, L), 0.0, 1.0);
    brdfInput.sqaureRoughness = model.roughness * model.roughness;
}

void BRDF(out vec3 fd, out vec3 fs, MicrofacetModel model, BRDFPrecomputedInput brdfInput)
{
    //Diffuse term, note that metallic objects doesn't have diffuse color
    fd = (1 - model.metallic) * 
    DisneyDiffuseFr(
        brdfInput.NdotV, 
        brdfInput.NdotL, 
        brdfInput.LdotH, 
        model.roughness ) 
    * INV_PI * model.albedo;

    //Specular term
    //Fresnel
    vec3 F = SchlickFr(brdfInput.LdotH, model.albedo, model.metallic);
    //Geometry occlusion from microfacet model(Visibility Height corellected)
    float G = SchlickGGX_SmithG_Vis_HC(brdfInput.NdotV, brdfInput.NdotL, brdfInput.sqaureRoughness);
    //NDF
    float D = SchlickGGX_D(brdfInput.NdotH, model.roughness);

    fs = F * G * D * INV_PI;
}

vec3 PointLightPass(PointLight light, MicrofacetModel model)
{
    vec3 unnormalizedL = light.position - model.pos;
	float d = length(unnormalizedL);
    model.L = unnormalizedL / d;
	
	BRDFPrecomputedInput brdfInput;
	ComputeBRDFInput(brdfInput, model);
    vec3 fd, fs;
    BRDF(fd, fs, model, brdfInput);
	
	//BRDF(fd, fs, model.N, model.V, model.L, model.albedo, model.metallic, model.roughness);

    float luminousIntensity = 0.25 * INV_PI * light.lumen;
    float distanceAttenuation = 1 / (d * d);
    float cosTheta = max(dot(model.L, model.N), 0.0);
	float illuminance = luminousIntensity * distanceAttenuation * cosTheta;

    return (fs + fd) * illuminance * light.RGBColor;
}
//vec3 AreaLightPass(MicrofacetModel model);

uniform sampler2D gBuffer0;
uniform sampler2D gBuffer1;
uniform sampler2D gBuffer2;

layout (std140, binding = 0) uniform ubCamera
{
    mat4 viewMat;
    mat4 projectionMat;
    vec3 camPos;
};

layout (std140, binding = 1) uniform ubSceneLights
{
	int numActivePointLights;
	int numActiveSphereLights;
    int pad1;
    int pad2;
	PointLight pointLights[ MAX_SCENE_POINT_LIGHT ];
	SphereLight sphereLights[ MAX_SCENE_SPHERE_LIGHT ];
};

out vec4 outColor;
in vec2 texCoord;

void main()
{    
    vec4 gBufferData0 = texture2D(gBuffer0, texCoord);
    vec4 gBufferData1 = texture2D(gBuffer1, texCoord);
    vec4 gBufferData2 = texture2D(gBuffer2, texCoord);

    vec3 fragPos = gBufferData1.rgb;

    MicrofacetModel model;

    model.albedo = gBufferData0.rgb;
    model.metallic = gBufferData0.a;
    model.N = normalize(gBufferData2.rgb); //microfacet model normal
    model.roughness = mix(0.02, 1.0, gBufferData1.a);
    model.V = normalize(camPos - fragPos); //view vector
    model.emission = gBufferData2.a;
    model.pos = fragPos;
    //model.L data is missing, when iterate through lights, fill it and compute input again,
    //todo lots of optimization could be done in here, only calculate view, light dependent variables
    
    vec3 Lo = vec3(0);
    for(int i = 0; i < numActivePointLights; ++i)
    {
        Lo += PointLightPass(pointLights[i], model);
    }

    Lo += model.albedo * model.emission;

	vec3 color = Lo;
	
	//HDR tonemapping
	color = color / (vec3(1.0)+color);
	
	//gamma correction
	color = pow( color, vec3(1.0 / 2.2));

    outColor = vec4( color, 1.0 );
} 