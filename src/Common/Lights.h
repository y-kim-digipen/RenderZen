#pragma once
#include <glm/vec3.hpp>

//Punctual light
struct PointLight
{
	glm::vec3 position;
	float lumen;
	glm::vec3 RGBColor;
	float radius;
};

//Area light
struct SphereLight
{
	glm::vec3 position;
	float lumen;
	glm::vec3 RGBColor;
	float radius;
};

//struct RectangularLight
//{
//	glm::vec3 position;
//	float lumen;
//	glm::vec3 RGBColor;
//	float radius;
//};
