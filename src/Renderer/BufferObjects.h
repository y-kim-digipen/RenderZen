#pragma once
#include <glm/glm.hpp>

#define MAX_SCENE_POINT_LIGHT 2
#define MAX_SCENE_SPHERE_LIGHT 2
struct PointLight
{
	glm::vec3 position;
	float lumen;
	glm::vec3 RGBColor;
	float radius;
};

struct SphereLight
{
	glm::vec3 position;
	float lumen;
	glm::vec3 RGBColor;
	float radius;
};

struct ubSceneLights
{
	int numActivePointLights;
	int numActiveSphereLights;
	int pad1;
	int pad2;
	PointLight pointLights[ MAX_SCENE_POINT_LIGHT ];
	SphereLight sphereLights[ MAX_SCENE_SPHERE_LIGHT ];
};

struct ubCamera
{
	glm::mat4 viewMat;
	glm::mat4 projectionMat;
	glm::vec3 camPos;
	float padding;
};

struct ubObject
{
	glm::mat4 toWorldMat;
	glm::vec4 albedoAlpha;
	float metallic;
	float roughness;
	float emission;
	float p1;
};