#pragma once
#include <glm/glm.hpp>
struct DirectionalLight
{
	glm::vec3 dir;
	float att;
	glm::vec3 color;
	float p2;
};