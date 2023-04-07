#version 450

layout (location = 0) out vec4 outAlbedo;
//position, metallic
layout (location = 1) out vec4 outPosition;
//normal, roughness
layout (location = 1) out vec4 outNormal;

// layout (location = 2) out vec4 gAlbedoSpec;

in VS_OUT
{
    vec4 viewPos;
	vec3 worldPos;
    vec3 normal;
	vec2 uv;
} FS_In;


void main()
{
    FragColor = vec4(FS_In.normal, 1.0);
} 