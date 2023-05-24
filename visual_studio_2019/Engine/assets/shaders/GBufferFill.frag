#version 450

layout (location = 0) out vec4 gBuffer0;
layout (location = 1) out vec4 gBuffer1;
layout (location = 2) out vec4 gBuffer2;


in VS_OUT
{
    vec3 viewPos;
	vec3 worldPos;
    vec3 normal;
	vec2 uv;
} FS_In;

layout (std140, binding = 1) uniform ubObject
{
    mat4 toWorldMat;
    vec4 albedoAlpha;
    float metallic;
    float roughness;
    float emission;
	float p1;
};

void main()
{
    gBuffer0 = vec4(albedoAlpha.rgb, metallic);
    gBuffer1 = vec4(FS_In.worldPos, roughness);
    gBuffer2 = vec4(FS_In.normal, emission);
} 