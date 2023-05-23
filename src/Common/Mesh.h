#pragma once
#include <vector>
#include <glm/glm.hpp>

struct Mesh
{
	std::vector<glm::vec3> mPositions;
	std::vector<glm::vec3> mNormals;
	std::vector<glm::vec2> mUVs;
	std::vector<uint32_t>  mIndices;
};