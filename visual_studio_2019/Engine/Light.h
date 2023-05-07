#pragma once
#include <glm/glm.hpp>

struct DirectionalLight
{
	glm::vec3 dir { 0, -1, 0 };
	float att {1.0};
	glm::vec3 color { 1.0, 0.2, 0.3 };
	bool edited;
};

