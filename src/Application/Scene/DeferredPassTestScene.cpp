#include "DeferredPassTestScene.h"
#include "../Application.h"
#include "../../Common/Mesh.h"
#include "../Components/Object.h"
#include "../../Renderer/Renderer.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include "imgui.h"


extern std::function<void ( int key, int action )> KeyCallback;

bool DeferredPassTestScene::Init ()
{	 return true;
}

bool DeferredPassTestScene::Load () {
	////////////
	pObjects.resize ( 3 );
	pObjects[ 0 ] = new Object;	 pObjects[ 0 ]->mGeometry.vaoID = gApplication.vaoID[ 0 ];	 pObjects[ 0 ]->mGeometry.numIndices = ( unsigned ) gApplication.pMeshes[ 0 ]->mIndices.size ();	 pObjects[ 0 ]->mTransform.mPos = glm::vec3 ( -10, 0, 0 );	 pObjects[ 0 ]->mTransform.mScale = glm::vec3 ( 1.f ), pObjects[0]->mTransform.mRot = glm::vec3(-glm::pi<float>() * 0.5f,0,0);
	pObjects[ 1 ] = new Object;	 pObjects[ 1 ]->mGeometry.vaoID = gApplication.vaoID[ 1 ];	 pObjects[ 1 ]->mGeometry.numIndices = ( unsigned ) gApplication.pMeshes[ 1 ]->mIndices.size ();	 pObjects[ 1 ]->mTransform.mPos = glm::vec3 ( 0, 0, 0 );	 pObjects[ 1 ]->mTransform.mScale = glm::vec3 ( 1.f );
	pObjects[ 2 ] = new Object;	 pObjects[ 2 ]->mGeometry.vaoID = gApplication.vaoID[ 2 ];	 pObjects[ 2 ]->mGeometry.numIndices = ( unsigned ) gApplication.pMeshes[ 2 ]->mIndices.size ();	 pObjects[ 2 ]->mTransform.mPos = glm::vec3 ( 10, 0, 0 );	 pObjects[ 2 ]->mTransform.mScale = glm::vec3 ( 1.f );
	pDeferredPassShaderProgram = gRenderer.pShaderPrograms[ 1 ];
	pCelShadingShaderProgram = gRenderer.pShaderPrograms[ 2 ];

	//Camera
	{
		glGenBuffers ( 1, &cameraUBOID );

		const int deferredPassShaderProgram = pDeferredPassShaderProgram->mHandle;

		int cbCameraLoc = glGetUniformBlockIndex ( pDeferredPassShaderProgram->mHandle, "cbCamera" );
		glUniformBlockBinding ( deferredPassShaderProgram, cbCameraLoc, 0 );
		glBindBuffer ( GL_UNIFORM_BUFFER, cameraUBOID );
		glBufferData ( GL_UNIFORM_BUFFER, sizeof ( cbCamera ), NULL, GL_STATIC_DRAW );
		glBindBuffer ( GL_UNIFORM_BUFFER, 0 );
		glBindBufferBase ( GL_UNIFORM_BUFFER, 0, cameraUBOID );

		cbCameraLoc = glGetUniformBlockIndex ( pCelShadingShaderProgram->mHandle, "cbCamera" );
		glUniformBlockBinding ( pCelShadingShaderProgram->mHandle, cbCameraLoc, 0 );
		glBindBuffer ( GL_UNIFORM_BUFFER, cameraUBOID );
		glBufferData ( GL_UNIFORM_BUFFER, sizeof ( cbCamera ), NULL, GL_STATIC_DRAW );
		glBindBuffer ( GL_UNIFORM_BUFFER, 0 );
	}

	//DirectionalLight
	{
		glGenBuffers ( 1, &directionalLightUBOID );

		int directionalLightLoc=  glGetUniformBlockIndex ( pCelShadingShaderProgram->mHandle, "DirectionalLight" );

		glUniformBlockBinding ( pCelShadingShaderProgram->mHandle, directionalLightLoc, 1 );
		glBindBuffer ( GL_UNIFORM_BUFFER, directionalLightUBOID );
		glBufferData ( GL_UNIFORM_BUFFER, sizeof ( DirectionalLight ), NULL, GL_STATIC_DRAW );
		glBindBuffer ( GL_UNIFORM_BUFFER, 1 );
		glBindBufferBase ( GL_UNIFORM_BUFFER, 1, directionalLightUBOID );

		//Update buffer


		//Set texture slots
		glUseProgram ( pCelShadingShaderProgram->mHandle );
		int loc =  glGetUniformLocation ( pCelShadingShaderProgram->mHandle, "gAlbedo" );
		glUniform1i ( loc, 0 ); // set it manually
		loc =  glGetUniformLocation ( pCelShadingShaderProgram->mHandle, "gPosition" );
		glUniform1i ( loc, 1 ); // set it manually
		loc =  glGetUniformLocation ( pCelShadingShaderProgram->mHandle, "gNormal" );
		glUniform1i ( loc, 2 ); // set it manually
		glUseProgram ( 0 );
	}


	LoadGUI ();
	LoadGBuffers ();

	glEnable ( GL_DEPTH_TEST );
	glEnable ( GL_CULL_FACE );
	glCullFace ( GL_BACK );
	glDepthFunc ( GL_LESS );
	

	glClearDepthf ( 1.0f );
	
	glGenVertexArrays ( 1, &emptyVAO );


	//Update uniform blocks

	glBindBuffer ( GL_UNIFORM_BUFFER, directionalLightUBOID );
	glBufferSubData ( GL_UNIFORM_BUFFER, 0, sizeof ( DirectionalLight ), &directionalLight );
	glBindBuffer ( GL_UNIFORM_BUFFER, 0 );

	glBindBuffer ( GL_UNIFORM_BUFFER, cameraUBOID );
	glBufferSubData ( GL_UNIFORM_BUFFER, 0, sizeof ( cbCamera ), &cameraController.gpuData );
	glBindBuffer ( GL_UNIFORM_BUFFER, 0 );

	return true;
}

void DeferredPassTestScene::Update ( float dt )
{
	glEnable ( GL_DEPTH_TEST );
	glEnable ( GL_CULL_FACE );
	glCullFace ( GL_BACK );
	glDepthFunc ( GL_LESS );

	cameraController.Update ( dt );

	if(cameraController.updated == false )
	{
		glBindBufferRange ( GL_UNIFORM_BUFFER, 0, cameraUBOID, 0, sizeof ( cbCamera ) );
		glBindBuffer ( GL_UNIFORM_BUFFER, cameraUBOID );
		glBufferSubData ( GL_UNIFORM_BUFFER, 0, sizeof ( cbCamera ), &cameraController.gpuData);
		glBindBuffer ( GL_UNIFORM_BUFFER, 0 );
		cameraController.updated = true;
	}

	glBindBuffer ( GL_UNIFORM_BUFFER, directionalLightUBOID );
	glBufferSubData ( GL_UNIFORM_BUFFER, 0, sizeof ( DirectionalLight ), &directionalLight );
	glBindBuffer ( GL_UNIFORM_BUFFER, 0 );
}

void DeferredPassTestScene::Draw () {
	//GBuffer fill pass
	{
		const int deferredPassShaderProgram = pDeferredPassShaderProgram->mHandle;
		glBindFramebuffer ( GL_DRAW_FRAMEBUFFER, FBO );

		glClearColor ( 0.f, 0.f, 0.f, 1.0f );
		glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

		glUseProgram ( deferredPassShaderProgram );

		//int UBOIndex = glGetUniformBlockIndex ( deferredPassShaderProgram, "cbCamera" );
		//glUniformBlockBinding ( forwardPassShaderProgram, UBOIndex, 0 );
		int numObjects = ( int ) pObjects.size ();
		for ( int i = 0; i < numObjects; ++i ) {
			Object* pObject = pObjects[ i ];
			Transform transform = pObject->mTransform;
			Geometry geometry = pObject->mGeometry;
			const int toWorldUniformLocation = glGetUniformLocation ( deferredPassShaderProgram, "toWorldMat" );
			const int albedoUniformLocation = glGetUniformLocation ( deferredPassShaderProgram, "albedo" );
			glm::mat4 toWorldMat = glm::mat4 ( 1 );
			glm::quat quat ( transform.mRot );
			toWorldMat = glm::translate ( toWorldMat, transform.mPos );
			toWorldMat = toWorldMat * glm::mat4 ( quat );
			toWorldMat = glm::scale ( toWorldMat, transform.mScale );
			glUniformMatrix4fv ( toWorldUniformLocation, 1, false, &toWorldMat[ 0 ][ 0 ] );
			glUniform3fv ( albedoUniformLocation, 1, &pObject->mMaterial.albedo.x );
			glBindVertexArray ( geometry.vaoID );

			glDrawElements ( GL_TRIANGLES, ( GLsizei ) geometry.numIndices, GL_UNSIGNED_INT, 0 );
		}
		glUseProgram ( 0 );
	}


	//Cel shading pass
	{
		glClearColor ( 0.3f, 0.5f, 0.2f, 1.0f );
		glBindFramebuffer ( GL_FRAMEBUFFER, 0 );
		glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

		//glBindFramebuffer ( GL_READ_FRAMEBUFFER, FBO );
		int shaderProgramHandle = pCelShadingShaderProgram->mHandle;
		//glReadBuffer ( GL_TEXTURE0 );
		//glReadBuffer ( GL_TEXTURE1 );
		//glReadBuffer ( GL_TEXTURE2 );
		glUseProgram ( shaderProgramHandle );

		glActiveTexture ( GL_TEXTURE0 );
		glBindTexture ( GL_TEXTURE_2D, positionTextureHandle );
		glActiveTexture ( GL_TEXTURE1 );
		glBindTexture ( GL_TEXTURE_2D, normalTextureHandle );
		glActiveTexture ( GL_TEXTURE2 );
		glBindTexture ( GL_TEXTURE_2D, colorTextureHandle );

		glBindVertexArray ( emptyVAO );
		glDrawArrays ( GL_TRIANGLES, 0, 3 );

		glUseProgram ( 0 );
	}

	//glBindFramebuffer ( GL_FRAMEBUFFER, 0 );
}

void DeferredPassTestScene::Unload ()
{	 
}

void DeferredPassTestScene::Cleanup ()
{	 
}

void DeferredPassTestScene::LoadGBuffers () {
	glGenFramebuffers ( 1, &FBO );
	glBindFramebuffer ( GL_FRAMEBUFFER, FBO );

	glGenTextures ( 1, &positionTextureHandle );
	glGenTextures ( 1, &normalTextureHandle );
	glGenTextures ( 1, &colorTextureHandle );
	glGenTextures ( 1, &depthTextureHandle );

	glBindTexture ( GL_TEXTURE_2D, positionTextureHandle );
	glTexImage2D ( GL_TEXTURE_2D, 0, GL_RGBA16F, 1920, 1080, 0, GL_RGBA, GL_FLOAT, NULL );
	glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glFramebufferTexture2D ( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, positionTextureHandle, 0 );

	glBindTexture ( GL_TEXTURE_2D, normalTextureHandle );
	glTexImage2D ( GL_TEXTURE_2D, 0, GL_RGBA16F, 1920, 1080, 0, GL_RGBA, GL_FLOAT, NULL );
	glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glFramebufferTexture2D ( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, normalTextureHandle, 0 );

	glBindTexture ( GL_TEXTURE_2D, colorTextureHandle );
	glTexImage2D ( GL_TEXTURE_2D, 0, GL_RGBA16F, 1920, 1080, 0, GL_RGBA, GL_FLOAT, NULL );
	glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glFramebufferTexture2D ( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, colorTextureHandle, 0 );

	//Depth buffer
	glBindTexture ( GL_TEXTURE_2D, depthTextureHandle );
	glTexImage2D ( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, 1920, 1080, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL );
	glFramebufferTexture2D ( GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTextureHandle, 0 );
	
	unsigned int attachments[ 3 ] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers ( 3, attachments );

	GLenum status = glCheckFramebufferStatus ( GL_FRAMEBUFFER );
	if ( status != GL_FRAMEBUFFER_COMPLETE ) {
		fprintf ( stderr, "Framebuffer Error. Status 0x%x\n", status );
	}

	glBindFramebuffer ( GL_FRAMEBUFFER, 0 );
}

bool DeferredPassTestScene::LoadShadowMap()
{
	glGenFramebuffers ( 1, &shadowMapFBO );
	glGenTextures ( 1, &shadowMapTex );

	glBindTexture ( GL_TEXTURE_2D, shadowMapTex );
	glTexImage2D ( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 1920, 1080, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL );
	glTexParameterf ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameterf ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTextureParameterf ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTextureParameterf ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

	glBindFramebuffer ( GL_FRAMEBUFFER, shadowMapFBO );
	glFramebufferTexture2D ( GL_FRAMEBUFFER, GL_DEPTH_COMPONENT, GL_TEXTURE_2D, shadowMapTex, 0 );
	glDrawBuffer ( GL_NONE );
	glReadBuffer ( GL_NONE );

	GLenum frameBufferStatus = glCheckFramebufferStatus ( GL_FRAMEBUFFER );
	if(frameBufferStatus != GL_FRAMEBUFFER_COMPLETE )
	{
		fprintf ( stderr, "Framebuffer Error. Status 0x%x\n", frameBufferStatus );
		return false;
	}
	return true;
}

void DeferredPassTestScene::LoadGUI ()
{
	auto RenderGUIFunc = [ & ] ()
	{
		ImGui::Begin ( "Scene", 0, ImGuiWindowFlags_AlwaysAutoResize );

		const int numObjects = ( int ) pObjects.size ();
		static int objectSelectID = 0;

		//ImGuiInputTextCallback
		ImGui::SeparatorText ( "Camera" );
		ImGui::InputFloat3 ( "pos", &cameraController.gpuData.camPos.x );

		if ( ImGui::InputInt ( "Object", &objectSelectID, 1, 5 ) ) {
			objectSelectID = std::min ( std::max ( objectSelectID, 0 ), numObjects - 1 );
		}

		if ( numObjects > 0 ) {
			Object* pSelectedObject = pObjects[ objectSelectID ];
			Transform& transform = pSelectedObject->mTransform;
			
			ImGui::SeparatorText ( "Transform" );
			ImGui::DragFloat3 ( "translate", &transform.mPos.x, 0.01f );
			ImGui::DragFloat3 ( "rotate", &transform.mRot.x, 0.01f );
			ImGui::DragFloat3 ( "scale", &transform.mScale.x, 0.01f );


			ImGui::SeparatorText ( "Material" );
			ImGui::ColorEdit3 ( "albedo", &pSelectedObject->mMaterial.albedo.x );

		}

		ImGui::End ();

		ImGui::Begin ( "DebugRT", 0 );

		ImVec2 itemRectSize = ImGui::GetItemRectSize ();
		const float itemSizeScalar = itemRectSize.x / 1920.f;


		ImVec2 itemSize = { 1920 * itemSizeScalar, 1080 * itemSizeScalar };

		ImGui::Image ( (void*)(intptr_t)positionTextureHandle, itemSize, {0, 1}, {1, 0});
		ImGui::Image ( ( void* ) ( intptr_t ) normalTextureHandle, itemSize, { 0, 1 }, { 1, 0 } );
		ImGui::Image ( ( void* ) ( intptr_t ) colorTextureHandle, itemSize, { 0, 1 }, { 1, 0 } );

		//ImGui::Image ( ( void* ) ( intptr_t ) depthTextureHandle, itemSize, { 0, 1 }, { 1, 0 } );
		ImGui::End ();

		{
			ImGui::Begin ( "Lights", 0 );
			ImGui::SeparatorText ( "Directional" );
			{
				bool edited = false;
				edited |= ImGui::ColorEdit3 ( "col", &directionalLight.color.r );
				edited |= ImGui::SliderFloat3 ( "dir", &directionalLight.dir.r, -1.f, 1.f );

				if( edited )
				{
					glBindBuffer ( GL_UNIFORM_BUFFER, directionalLightUBOID );
					glBufferSubData ( GL_UNIFORM_BUFFER, 0, sizeof ( DirectionalLight ), &directionalLight );
					glBindBuffer ( GL_UNIFORM_BUFFER, 0 );
				}
			}

			ImGui::SeparatorText ( "Point" );
		}


		ImGui::End ();
	};

	RegisterGUIUpdateCallback ( RenderGUIFunc );
}
