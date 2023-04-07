#pragma once
#include <string>
#include <vector>

#include <glm/glm.hpp>

#define MAX_VERTEXATTRIB_LAYOUTS 10

struct MeshData
{
	bool hasPosition;
	bool hasNormal;
	bool hasUV;
	bool hasIndex;
	uint32_t mNumVertices;
	uint32_t mNumIndices;
};

struct Mesh
{
	std::vector<glm::vec3> mPositions;
	std::vector<glm::vec3> mNormals;
	std::vector<glm::vec2> mUVs;
	std::vector<uint32_t>  mIndices;

	MeshData mData{};
};

void SetMeshDirectory(const char* pDirectory);
bool CPULoadMesh(const char* pFile, Mesh* pMesh);