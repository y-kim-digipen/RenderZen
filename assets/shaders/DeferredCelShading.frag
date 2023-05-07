#version 450

uniform sampler2D gAlbedo;
uniform sampler2D gPosition;
uniform sampler2D gNormal;

layout (std140, binding = 0) uniform cbCamera
{
    mat4 viewMat;
    mat4 projectionMat;
    vec3 camPos;
};

layout(std140, binding = 1) uniform DirectionalLight 
{
    vec3 dl_dir;
	float dl_att;
    vec3 dl_color;
	bool dl_pad;
};

out vec4 outColor;
in vec2 texCoord;

const float step1 = 0.1;
const float step2 = 0.3;
const float step3 = 0.6;
const float step4 = 1.0;

const float glosiness = 16;

const float lightIntensity = 1.0;
const float rimAmount = 0.716;
const float rimThreshold = 1;

void main()
{    
    float AO = 0.2;

    vec3 albedo = texture(gAlbedo, texCoord).rgb;
    //in world space
    vec3 fragPosition = texture(gPosition, texCoord).rgb;
    vec3 N = normalize(texture(gNormal, texCoord).rgb);

    //fragColor += AO * albedo;

    float lightDiffuseAmount = 0.0;
    vec3 lightSpec = vec3(0.0);
    //Directional lighting
    {
        vec3 L = normalize(-dl_dir);
        vec3 V = normalize(camPos - fragPosition);
        vec3 H = normalize(L + V);

        float NdotL = dot(N, L);
        float NdotH = dot(N, H);

        float diffuse = smoothstep(0.0, 0.01, NdotL);
        vec3 diffuseColor = (diffuse * albedo); 

        //specular
        float spec = pow(NdotH * lightIntensity, glosiness * glosiness);
        float specularSmooth = smoothstep(0.005, 0.01, spec);
        vec3 specular = dl_color * specularSmooth;

        lightDiffuseAmount += diffuse;
        lightSpec += specular;

        //rim ring
        //float rimDot = 1.0 - dot(V, N);
        //float rimIntensity = smoothstep(rimAmount - 0.01, rimAmount + 0.01, rimDot);
        //lightDiffuseAmount += rimIntensity;

        //rim lighting
        float rimDot = 1.0 - dot(V, N);
        float rimIntensity = rimDot * pow ( NdotL, rimThreshold );
        rimIntensity = smoothstep(rimAmount - 0.01, rimAmount + 0.01, rimIntensity);
        lightDiffuseAmount += rimIntensity;
    }

    vec3 fragColor = vec3( AO + lightDiffuseAmount ) * albedo + lightSpec;
    fragColor = pow(fragColor, vec3(1.0 / 2.2));
    outColor = vec4(fragColor, 1.0);
} 