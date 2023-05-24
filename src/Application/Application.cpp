#include "Application.h"

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output mData structure
#include <assimp/postprocess.h>     // Post processing flags

#include "../Common/Mesh.h"
#include "../Renderer/Renderer.h"

#include <GLAD/glad.h>
#include <GLFW/glfw3.h>

#include "Scene/PBRLightingScene.h"
#include "../Tools/Input/Input.h"

Application gApplication;

Mesh* LoadMesh ( const char* meshPath ) {
    // Create an instance of the Importer class
    Assimp::Importer importer;

    // And have it read the given file with some example postprocessing
    // Usually - if speed is not the most important aspect for you - you'll
    // probably to request more postprocessing than we do in this example.
    const aiScene* scene = importer.ReadFile ( meshPath,
                                               aiProcess_CalcTangentSpace |
                                               aiProcess_Triangulate |
                                               aiProcess_JoinIdenticalVertices |
                                               aiProcess_SortByPType );

    Mesh* pMesh = new Mesh;

    // If the import failed, report it

    if ( !scene ) {
        //todo just for debugging
        assert ( false );
        return nullptr;
    }

    //struct MeshEntry
    //{

    //    uint32_t vertexCount;
    //    uint32_t indexOffset;
    //};

    for ( uint32_t i = 0, numMeshes = scene->mNumMeshes; i < numMeshes; ++i ) {
        aiMesh* pAiMesh = scene->mMeshes[ i ];

        bool hasPosition = pAiMesh->HasPositions ();
        bool hasNormal = pAiMesh->HasNormals ();
        bool hasUV = pAiMesh->HasTextureCoords ( 0 );
        bool hasIndex = pAiMesh->HasFaces ();

        assert ( hasPosition && hasNormal );
        assert ( hasIndex );

        unsigned mNumVertices = pAiMesh->mNumVertices;

        //Handling vertex
        for(uint32_t vertexIndex = 0; vertexIndex < mNumVertices; ++vertexIndex )
        {
            glm::vec3 vPosition = *( glm::vec3* ) &pAiMesh->mVertices[ vertexIndex ].x;
            pMesh->mPositions.push_back ( vPosition );
            glm::vec3 vNormal = *( glm::vec3* ) &pAiMesh->mNormals[ vertexIndex ].x;
            pMesh->mNormals.push_back ( vNormal );
        }
        
        if ( hasUV ) {
            for ( uint32_t vertexIndex = 0; vertexIndex < mNumVertices; ++vertexIndex ) {
                glm::vec2 uv = *((glm::vec2* ) & (pAiMesh->mTextureCoords[0]->x ));
                pMesh->mUVs.push_back ( uv );
            }
        }
        
        for ( uint32_t j = 0; j < pAiMesh->mNumFaces; ++j ) {
            aiFace& face = pAiMesh->mFaces[ j ];
            assert ( face.mNumIndices == 3 );
            pMesh->mIndices.push_back ( face.mIndices[ 0 ] );
            pMesh->mIndices.push_back ( face.mIndices[ 1 ] );
            pMesh->mIndices.push_back ( face.mIndices[ 2 ] );
        }
    }

    return pMesh;
}

bool Application::Init () {
    //pScene = new M1Scene();
    pScene = new DeferredPassTestScene;

    return pScene->Init ();
}

bool Application::Load () {
    const char* meshPath[] {
    "assets/meshes/cube.gltf",
    "assets/meshes/sphere.gltf",
    //"assets/meshes/cube.gltf",
    "assets/meshes/capsule.gltf" };

    Mesh* pMesh = NULL;
    for ( int i = 0; i < 3; ++i ) {
        pMesh = LoadMesh ( meshPath[ i ] );
        pMeshes.push_back ( pMesh );

        VAOLoadDesc vaoLoadDesc = MakeVAOLoadDesc ( pMesh );
        unsigned vao = gRenderer.LoadVAO ( vaoLoadDesc );
        vaoID.push_back ( vao );
    }

    bool sceneLoadRes = pScene->Load ();
    if ( !sceneLoadRes ) return false;
    mAppInfo.mScreenSize = { 1920, 1080 };
    return true;
    //pGeometries.
}

bool Application::ShouldUpdate () {
    return true;
}

static float lastTime = 0;
void Application::Update () {
    glfwPollEvents ();
    float curTime = ( float ) glfwGetTime () ;
    pScene->Update ( curTime - lastTime );
    lastTime = curTime;
}

void Application::Draw () {
    pScene->Draw ();
    Input::Input::Update ();
}

void Application::Cleanup () {
    pScene->Cleanup ();
}

void Application::Unload () {
    pScene->Unload ();
}

ApplicationInfo Application::GetAppInfo () {
    return mAppInfo;
}
