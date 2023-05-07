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
	glm::vec3 albedo;
	bool doUseAlbedoTexture;
	//albedoTextureHandle
};

struct Object
{
	Geometry mGeometry{};
	Material mMaterial{};
	Transform mTransform{};
};