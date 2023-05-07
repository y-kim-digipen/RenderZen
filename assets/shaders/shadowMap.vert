#version 450 core
layout (location = 0) in vec3 vPos;
layout (location = 1) in vec3 vNorm;
layout (location = 2) in vec2 vUV;

layout (std140, binding = 0) uniform cbCamera
{
    mat4 viewMat;
    mat4 projectionMat;
    vec3 camPos;
};

//layout (std140) uniform cbObject
//{
//    mat4 toWorldMat1;
//    vec3 albedo;
//    bool doUseAlbedoTexture;
//}

out VS_OUT
{
    vec4 viewPos;
	vec3 worldPos;
    vec3 normal;
	vec2 uv;
} vs_out;

uniform mat4 toWorldMat;
uniform vec3 albedo;

void main()
{
    vs_out.viewPos = projectionMat * viewMat * toWorldMat * vec4(vPos, 1.0);
    gl_Position = vec4(vs_out.viewPos);
    vs_out.worldPos = (toWorldMat * vec4(vPos, 1.0)).xyz;
    vs_out.normal = (toWorldMat * vec4(vNorm, 0.0)).xyz;
    vs_out.uv = vUV; 
}