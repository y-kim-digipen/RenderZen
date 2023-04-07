#version 450
out vec4 FragColor;

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