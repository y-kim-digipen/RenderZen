#include "AssetLoader.h"

#include <cassert>
#include <iostream>
#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output mData structure
#include <assimp/postprocess.h>     // Post processing flags

std::string gDirectory;

void SetMeshDirectory(const char* pDirectory)
{
    gDirectory = pDirectory;
}

bool AIProcessScene(const aiScene* scene, Mesh* pMesh)
{
    //todo handling just one mesh for scene now
    for(uint32_t i = 0, numMeshes = /*scene->mNumMeshes*/1; i < numMeshes; ++i)
    {
    	aiMesh* pAiMesh = scene->mMeshes[i];
        
    	pMesh->mData.hasPosition = pAiMesh->HasPositions();
        pMesh->mData.hasNormal = pAiMesh->HasNormals();
        pMesh->mData.hasUV = pAiMesh->HasTextureCoords(0);
        pMesh->mData.hasIndex = pAiMesh->HasFaces();

        assert(pMesh->mData.hasPosition && pMesh->mData.hasNormal);
        assert(pMesh->mData.hasIndex);

        pMesh->mData.mNumVertices = pAiMesh->mNumVertices;
        pMesh->mPositions.resize(pAiMesh->mNumVertices);
        pMesh->mNormals.resize(pAiMesh->mNumVertices);

        std::memcpy(pMesh->mPositions.data(), pAiMesh->mVertices, sizeof(glm::vec3) * pMesh->mData.mNumVertices);
        std::memcpy(pMesh->mNormals.data(), pAiMesh->mNormals, sizeof(glm::vec3) * pMesh->mData.mNumVertices);

        if (pMesh->mData.hasUV) {
            pMesh->mUVs.resize(pAiMesh->mNumVertices);
            std::memcpy(pMesh->mUVs.data(), pAiMesh->mTextureCoords[0], sizeof(glm::vec2) * pMesh->mData.mNumVertices);
        }

        pMesh->mData.mNumIndices = pAiMesh->mNumFaces * 3;
        pMesh->mIndices.resize(pMesh->mData.mNumIndices);
        for(uint32_t j = 0 ; j < pAiMesh->mNumFaces; ++j)
        {
            aiFace& face = pAiMesh->mFaces[j];
            assert(face.mNumIndices == 3);
            std::memcpy(&pMesh->mIndices[j * 3], face.mIndices, sizeof(uint32_t) * 3);
        }
    }
    return true;
}

bool CPULoadMesh(const char* pFile, Mesh* pMesh)
{
    assert(pMesh);
    assert(!gDirectory.empty());

	std::string assetPath = gDirectory + "/" + pFile;

    // Create an instance of the Importer class
    Assimp::Importer importer;

    // And have it read the given file with some example postprocessing
    // Usually - if speed is not the most important aspect for you - you'll
    // probably to request more postprocessing than we do in this example.
    const aiScene* scene = importer.ReadFile(assetPath.c_str(),
        aiProcess_CalcTangentSpace |
        aiProcess_Triangulate |
        aiProcess_JoinIdenticalVertices |
        aiProcess_SortByPType);

    // If the import failed, report it
    if (nullptr == scene) {
        //todo just for debugging
        assert(false);
    	return false;
    }
    
    AIProcessScene(scene, pMesh);
    // Now we can access the file's contents.
    //DoTheSceneProcessing(scene);

    // We're done. Everything will be cleaned up by the importer destructor

	return true;
}

