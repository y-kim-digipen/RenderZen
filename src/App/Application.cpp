#include "Application.h"

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output mData structure
#include <assimp/postprocess.h>     // Post processing flags

#include "Mesh.h"
#include "Renderer.h"

#include <GLAD/glad.h>
#include <GLFW/glfw3.h>

#include "M1Scene.h"
#include "DeferredPassTestScene.h"
#include "Input.h"

Application gApplication;

Mesh* LoadMesh(const char* meshPath)
{
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

    for ( uint32_t i = 0, numMeshes = /*scene->mNumMeshes*/1; i < numMeshes; ++i ) {
        aiMesh* pAiMesh = scene->mMeshes[ i ];

        bool hasPosition = pAiMesh->HasPositions ();
        bool hasNormal = pAiMesh->HasNormals ();
        bool hasUV = pAiMesh->HasTextureCoords ( 0 );
        bool hasIndex = pAiMesh->HasFaces ();

        assert ( hasPosition && hasNormal );
        assert ( hasIndex );

        unsigned mNumVertices = pAiMesh->mNumVertices;
        pMesh->mPositions.resize ( pAiMesh->mNumVertices );
        pMesh->mNormals.resize ( pAiMesh->mNumVertices );

        std::memcpy ( pMesh->mPositions.data (), pAiMesh->mVertices, sizeof ( glm::vec3 ) * mNumVertices );
        std::memcpy ( pMesh->mNormals.data (), pAiMesh->mNormals, sizeof ( glm::vec3 ) * mNumVertices );

        if ( hasUV ) {
            pMesh->mUVs.resize ( pAiMesh->mNumVertices );
            std::memcpy ( pMesh->mUVs.data (), pAiMesh->mTextureCoords[ 0 ], sizeof ( glm::vec2 ) * mNumVertices );
        }

        unsigned mNumIndices = pAiMesh->mNumFaces * 3;
        pMesh->mIndices.resize ( mNumIndices );
        for ( uint32_t j = 0; j < pAiMesh->mNumFaces; ++j ) {
            aiFace& face = pAiMesh->mFaces[ j ];
            assert ( face.mNumIndices == 3 );
            std::memcpy ( &pMesh->mIndices[ j * 3 ], face.mIndices, sizeof ( uint32_t ) * 3 );
        }
    }
    // Now we can access the file's contents.
    //DoTheSceneProcessing(scene);

    // We're done. Everything will be cleaned up by the importer destructor

    return pMesh;
}

bool Application::Init()
{
    //pScene = new M1Scene();
    pScene = new DeferredPassTestScene;

    return pScene->Init ();
}

bool Application::Load()
{
    const char* meshPath[] {
	"assets/meshes/suzanne.fbx",
    "assets/meshes/sphere.gltf",
    //"assets/meshes/cube.gltf",
	"assets/meshes/capsule.gltf" };

    Mesh* pMesh = NULL;
    for(int i = 0; i < 3; ++i )
    {
        pMesh = LoadMesh (meshPath[i]);
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

bool Application::ShouldUpdate()
{
    return true;
}

void Application::Update( )
{
    glfwPollEvents ();
    float t = (float)glfwGetTime ();
	pScene->Update ( t );
}

void Application::Draw()
{
    pScene->Draw ();
    Input::Input::Update ();
}

void Application::Cleanup()
{
    pScene->Cleanup ();
}

void Application::Unload()
{
    pScene->Unload ();
}

ApplicationInfo Application::GetAppInfo()
{
    return mAppInfo;
}
