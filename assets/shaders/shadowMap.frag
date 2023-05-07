#version 450

layout (location = 0) out vec4 outAlbedo;
//position, metallic
layout (location = 1) out vec4 outPosition;
//normal, roughness
layout (location = 2) out vec4 outNormal;

// layout (location = 2) out vec4 gAlbedoSpec;

layout (std140, binding = 1) uniform DirectionalLight
{
	vec3 color;
	float att;
	vec3 dir;
	float p2;
};

in VS_OUT
{
    vec4 viewPos;
	vec3 worldPos;
    vec3 normal;
	vec2 uv;
} FS_In;

uniform vec3 albedo;

void main()
{
    outAlbedo = vec4(albedo, 1.0);
    outPosition = vec4(FS_In.worldPos, 1.0);
    outNormal = vec4(FS_In.normal, 1.0);
} 