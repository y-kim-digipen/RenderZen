#version 450
out vec4 FragColor;

in VS_OUT
{
    vec4 viewPos;
	vec3 worldPos;
    vec3 normal;
	vec2 uv;
} FS_In;

uniform vec4 emissionColor;

void main()
{
    FragColor = emissionColor;
}