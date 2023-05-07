#include "M1Scene.h"
#include "Application.h"
#include "Mesh.h"
#include "Object.h"
#include "Renderer.h"

#include <glad/glad.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include "imgui.h"

bool M1Scene::Init()
{
	return true;
}

bool M1Scene::Load()
{
	glm::vec3 cameraPos = glm::vec3 ( 0.0f, 0.0f, 30.0f );
	glm::vec3 cameraFront = glm::vec3 ( 0.0f, 0.0f, -1.0f );
	glm::vec3 cameraUp = glm::vec3 ( 0.0f, 1.0f, 0.0f );

	camera.viewMat = lookAt ( cameraPos, cameraPos + cameraFront, cameraUp );
	camera.projectionMat = glm::perspective ( glm::radians ( 45.0f ), ( float ) 1920 / ( float ) 1080, 0.1f, 100.0f );
	camera.camPos = { 0, 0, 3 };

	////////////

	pObjects.resize(3);
	pObjects[ 0 ] = new Object;
	pObjects[ 0 ]->mGeometry.vaoID = gApplication.vaoID[ 0 ];
	pObjects[ 0 ]->mGeometry.numIndices = ( unsigned ) gApplication.pMeshes[ 0 ]->mIndices.size ();
	pObjects[ 0 ]->mTransform.mPos = glm::vec3 ( -10, 0, 0 );
	pObjects[ 0 ]->mTransform.mScale = glm::vec3 ( 0.01f );

	pObjects[ 1 ] = new Object;
	pObjects[ 1 ]->mGeometry.vaoID = gApplication.vaoID[ 1 ];
	pObjects[ 1 ]->mGeometry.numIndices = ( unsigned ) gApplication.pMeshes[ 1 ]->mIndices.size ();
	pObjects[ 1 ]->mTransform.mPos = glm::vec3 ( 0, 0, 0 );
	pObjects[ 1 ]->mTransform.mScale = glm::vec3 ( 1.f );

	pObjects[ 2 ] = new Object;
	pObjects[ 2 ]->mGeometry.vaoID = gApplication.vaoID[ 2 ];
	pObjects[ 2 ]->mGeometry.numIndices = ( unsigned ) gApplication.pMeshes[ 2 ]->mIndices.size ();
	pObjects[ 2 ]->mTransform.mPos = glm::vec3 ( 10, 0, 0 );
	pObjects[ 2 ]->mTransform.mScale = glm::vec3 ( 1.f );

	pForwardPassShaderProgram = gRenderer.pShaderPrograms[ 0 ];
	const int forwardPassShaderProgram = pForwardPassShaderProgram->mHandle;
	////////////
	{
		
	}
	int uniformBlockIndexRed     = glGetUniformBlockIndex ( forwardPassShaderProgram, "cbCamera" );
	glUniformBlockBinding ( forwardPassShaderProgram, uniformBlockIndexRed, 0 );

	unsigned int uboMatrices;
	glGenBuffers ( 1, &uboMatrices );

	glBindBuffer ( GL_UNIFORM_BUFFER, uboMatrices );
	glBufferData ( GL_UNIFORM_BUFFER, sizeof( cbCamera ), NULL, GL_STATIC_DRAW);
	glBindBuffer ( GL_UNIFORM_BUFFER, 0 );

	glBindBufferRange ( GL_UNIFORM_BUFFER, 0, uboMatrices, 0, sizeof (cbCamera ) );

	glBindBuffer ( GL_UNIFORM_BUFFER, uboMatrices );
	glBufferSubData ( GL_UNIFORM_BUFFER, 0, sizeof ( cbCamera ), &camera );
	glBindBuffer ( GL_UNIFORM_BUFFER, 0 );

	LoadGUI ();

	return true;
}

void M1Scene::Update( float dt )
{
	pObjects[ 0 ]->mTransform.mRot.y += 0.001f;
}

void M1Scene::Draw()
{
	const int forwardPassShaderProgram = pForwardPassShaderProgram->mHandle;

	glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	glUseProgram ( forwardPassShaderProgram );

	int UBOIndex = glGetUniformBlockIndex ( forwardPassShaderProgram, "cbCamera" );
	//glUniformBlockBinding ( forwardPassShaderProgram, UBOIndex, 0 );

	int numObjects = (int)pObjects.size ();
	for( int i = 0; i < numObjects; ++i )
	{
		Object* pObject = pObjects[ i ];
		Transform transform = pObject->mTransform;

		Geometry geometry = pObject->mGeometry;

		const int uniformLoc = glGetUniformLocation ( forwardPassShaderProgram, "toWorldMat" );
		glm::mat4 toWorldMat = glm::mat4 ( 1 );
		glm::quat quat ( transform.mRot );

		toWorldMat = glm::translate ( toWorldMat, transform.mPos );
		toWorldMat = toWorldMat * glm::mat4 ( quat );
		toWorldMat = glm::scale ( toWorldMat, transform.mScale );
		
		glUniformMatrix4fv ( uniformLoc, 1, false, &toWorldMat[ 0 ][ 0 ] );

		glBindVertexArray ( geometry.vaoID );
		glDrawElements ( GL_TRIANGLES, ( GLsizei ) geometry.numIndices, GL_UNSIGNED_INT, 0 );
	}

	glUseProgram ( 0 );
}

void M1Scene::Unload()
{
}

void M1Scene::Cleanup()
{
}

void M1Scene::LoadGUI()
{
	auto RenderGUIFunc = [ & ] ()
	{
		ImGui::Begin ( "Scene", 0, ImGuiWindowFlags_AlwaysAutoResize );

		const int numObjects = ( int ) pObjects.size ();
		static int objectSelectID = 0;

		//ImGuiInputTextCallback
		if(ImGui::InputInt("Object", &objectSelectID, 1, 5 ))
		{
			objectSelectID = std::min ( std::max ( objectSelectID, 0 ), numObjects - 1 );
		}

		if(numObjects > 0 )
		{
			Object* pSelectedObject = pObjects[ objectSelectID ];
			Transform& transform = pSelectedObject->mTransform;

			ImGui::SeparatorText ( "Transform" );
			ImGui::DragFloat3 ( "translate", &transform.mPos.x, 0.01f );
			ImGui::DragFloat3 ( "rotate", &transform.mRot.x, 0.01f );
			ImGui::DragFloat3 ( "scale", &transform.mScale.x, 0.01f );
		}

		ImGui::End ();
	};

	RegisterGUIUpdateCallback ( RenderGUIFunc );
}
