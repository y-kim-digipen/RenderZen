#version 450 core
layout (location = 0) in vec3 vPos;
layout (location = 1) in vec3 vNorm;
layout (location = 2) in vec2 vUV;

layout (std140, binding = 0) uniform ubCamera
{
    mat4 viewMat;
    mat4 projectionMat;
    vec3 camPos;
};

layout (std140, binding = 1) uniform ubObject
{
    mat4 toWorldMat;
    vec4 albedoAlpha;
    float metallic;
    float roughness;
    float emission;
	float p1;
};

out VS_OUT
{
    vec3 viewPos;
	vec3 worldPos;
    vec3 normal;
	vec2 uv;
} vs_out;

void main()
{
    vec4 worldPos = toWorldMat * vec4(vPos, 1.0);
    vec4 viewPos = viewMat * worldPos;

    vs_out.viewPos = viewPos.xyz;
    vs_out.worldPos = worldPos.xyz;
    gl_Position = projectionMat * viewPos;

    vec4 normal = (toWorldMat * vec4(vNorm, 0.0));
    vs_out.normal = normalize(normal.xyz);
    vs_out.uv = vUV;
}