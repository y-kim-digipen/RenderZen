#pragma once
#include <glm/glm.hpp>
#include "Geometry.h"

struct Transform
{
	glm::vec3 mPos{0};
	glm::vec3 mRot{0};
	glm::vec3 mScale{1};
};

struct Material
{
	glm::vec3 albedo{0.5f};
	float metallic{0.0f};
	float roughness{0.04f};
	float emission{0.1f};
};

struct Object
{
	Geometry mGeometry{};
	Material mMaterial{};
	Transform mTransform{};
};