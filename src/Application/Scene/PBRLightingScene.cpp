#include "PBRLightingScene.h"
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
	pObjects[ 0 ] = new Object;
	pObjects[ 0 ]->mGeometry.vaoID = gApplication.vaoID[ 0 ];
	pObjects[ 0 ]->mGeometry.numIndices = ( unsigned ) gApplication.pMeshes[ 0 ]->mIndices.size ();
	pObjects[ 0 ]->mTransform.mPos = glm::vec3 ( -10, 0, 0 );
	pObjects[ 0 ]->mTransform.mScale = glm::vec3 ( 1.f );
	pObjects[ 0 ]->mTransform.mRot = glm::vec3 ( -glm::pi<float> () * 0.5f, 0, 0 );
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

	pGBufferFillShaderProgram = gRenderer.pShaderPrograms[ 1 ];
	pBRDFLightingShaderProgram = gRenderer.pShaderPrograms[ 2 ];

	LoadGBuffers ();

	//Load UBOs
	int slot = 0;
	//Camera
	{
		glGenBuffers ( 1, &cameraUBO );

		glBindBuffer ( GL_UNIFORM_BUFFER, cameraUBO );
		glBufferData ( GL_UNIFORM_BUFFER, sizeof ( ubCamera ), NULL, GL_STATIC_DRAW );
		glBindBuffer ( GL_UNIFORM_BUFFER, 0 );
		glBindBufferBase ( GL_UNIFORM_BUFFER, slot, cameraUBO );


		int loc = glGetUniformBlockIndex ( pGBufferFillShaderProgram->mHandle, "ubCamera" );
		glUniformBlockBinding ( pGBufferFillShaderProgram->mHandle, loc, slot );

		loc = glGetUniformBlockIndex ( pBRDFLightingShaderProgram->mHandle, "ubCamera" );
		glUniformBlockBinding ( pBRDFLightingShaderProgram->mHandle, loc, slot );
	}

	slot++;
	//Object
	{
		glGenBuffers ( 1, &objectUBO );
		glBindBuffer ( GL_UNIFORM_BUFFER, objectUBO );
		glBufferData ( GL_UNIFORM_BUFFER, sizeof ( ubObject ), NULL, GL_STATIC_DRAW );
		glBindBuffer ( GL_UNIFORM_BUFFER, 0 );
		glBindBufferBase ( GL_UNIFORM_BUFFER, slot, objectUBO );

		int loc =  glGetUniformBlockIndex ( pGBufferFillShaderProgram->mHandle, "ubObject" );
		glUniformBlockBinding ( pGBufferFillShaderProgram->mHandle, loc, slot );

		//Update buffer
	}

	slot++;
	//SceneLights
	{
		glGenBuffers ( 1, &sceneLightsUBO );
		glBindBuffer ( GL_UNIFORM_BUFFER, sceneLightsUBO );
		glBufferData ( GL_UNIFORM_BUFFER, sizeof ( ubSceneLights ), NULL, GL_STATIC_DRAW );
		glBindBuffer ( GL_UNIFORM_BUFFER, 0 );
		glBindBufferBase ( GL_UNIFORM_BUFFER, slot, sceneLightsUBO );

		int loc = glGetUniformBlockIndex ( pBRDFLightingShaderProgram->mHandle, "ubSceneLights" );
		glUniformBlockBinding ( pBRDFLightingShaderProgram->mHandle, loc, slot );
	}

	//linking textures
	{
		int shaderProgram = pBRDFLightingShaderProgram->mHandle;

		glUseProgram ( shaderProgram );

		int uniformLocation = glGetUniformLocation ( shaderProgram, "gBuffer0" );
		glUniform1i ( uniformLocation, 0 );
		uniformLocation = glGetUniformLocation ( shaderProgram, "gBuffer1" );
		glUniform1i ( uniformLocation, 1 );
		uniformLocation = glGetUniformLocation ( shaderProgram, "gBuffer2" );
		glUniform1i ( uniformLocation, 2 );

		glUseProgram ( 0 );
	}

	LoadGUI ();

	glEnable ( GL_DEPTH_TEST );
	glEnable ( GL_CULL_FACE );
	glCullFace ( GL_BACK );
	glDepthFunc ( GL_LESS );

	glClearColor ( 0.0f, 0.0f, 0.0f, 0.0f );
	//glClearDepthf ( 1.0f );

	sceneLights.numActivePointLights = 1;
	PointLight& pointLight = sceneLights.pointLights[ 0 ];
	pointLight.position = { 0, 2, 0 };
	pointLight.RGBColor = { 0.78, 0.8, 0.93 };
	pointLight.lumen = 700;
	//pointLight.

	//Update uniform blocks
	glBindBuffer ( GL_UNIFORM_BUFFER, cameraUBO );
	glBufferSubData ( GL_UNIFORM_BUFFER, 0, sizeof ( ubCamera ), &cameraController.gpuData );
	glBindBuffer ( GL_UNIFORM_BUFFER, 0 );

	glBindBuffer ( GL_UNIFORM_BUFFER, sceneLightsUBO );
	glBufferSubData ( GL_UNIFORM_BUFFER, 0, sizeof ( ubSceneLights ), &sceneLights );
	glBindBuffer ( GL_UNIFORM_BUFFER, 0 );

	glGenVertexArrays ( 1, &emptyVAO );
	
	return true;
}

void DeferredPassTestScene::Update ( float dt )
{
	cameraController.Update ( dt );

	if(cameraController.updated == false )
	{
		//glBindBufferRange ( GL_UNIFORM_BUFFER, 0, cameraUBO, 0, sizeof ( ubCamera ) );
		glBindBuffer ( GL_UNIFORM_BUFFER, cameraUBO );
		glBufferSubData ( GL_UNIFORM_BUFFER, 0, sizeof ( ubCamera ), &cameraController.gpuData);
		glBindBuffer ( GL_UNIFORM_BUFFER, 0 );
		cameraController.updated = true;
	}

	//glBindBuffer ( GL_UNIFORM_BUFFER, objectUBO );
	//glBufferSubData ( GL_UNIFORM_BUFFER, 0, sizeof ( DirectionalLight ), &directionalLight );
	//glBindBuffer ( GL_UNIFORM_BUFFER, 0 );
}

void DeferredPassTestScene::Draw () {
	//GBuffer fill pass
	glBindFramebuffer ( GL_FRAMEBUFFER, gBufferFillFBO );
	glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	{
		const int shaderProgram = pGBufferFillShaderProgram->mHandle;

		glUseProgram ( shaderProgram );
		
		int numObjects = ( int ) pObjects.size ();
		for ( int i = 0; i < numObjects; ++i ) {
			Object* pObject = pObjects[ i ];
			Transform transform = pObject->mTransform;
			Geometry geometry = pObject->mGeometry;


			glm::mat4 toWorldMat = glm::mat4 ( 1 );
			glm::quat quat ( transform.mRot );
			toWorldMat = glm::translate ( toWorldMat, transform.mPos );
			toWorldMat = toWorldMat * glm::mat4 ( quat );
			toWorldMat = glm::scale ( toWorldMat, transform.mScale );

			ubObject uniformBlockObject{};


			uniformBlockObject.toWorldMat = toWorldMat;
			*(glm::vec3*)&uniformBlockObject.albedoAlpha.r = pObject->mMaterial.albedo;
			uniformBlockObject.metallic = pObject->mMaterial.metallic;
			uniformBlockObject.roughness = pObject->mMaterial.roughness;
			uniformBlockObject.emission = pObject->mMaterial.emission;

			glBindBuffer ( GL_UNIFORM_BUFFER, objectUBO );
			glBufferSubData ( GL_UNIFORM_BUFFER, 0, sizeof ( ubObject ), &uniformBlockObject );
			glBindBuffer ( GL_UNIFORM_BUFFER, 0 );


			glBindVertexArray ( geometry.vaoID );
			glDrawElements ( GL_TRIANGLES, ( GLsizei ) geometry.numIndices, GL_UNSIGNED_INT, 0 );
		}

		glUseProgram ( 0 );
	}

	glBindFramebuffer ( GL_FRAMEBUFFER, 0 );
	
	glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	//BRDF shading pass
	{
		int shaderProgramHandle = pBRDFLightingShaderProgram->mHandle;

		glUseProgram ( shaderProgramHandle );

		//glDisable ( GL_DEPTH_TEST );

		glActiveTexture ( GL_TEXTURE0 );
		glBindTexture ( GL_TEXTURE_2D, gBuffer0 );
		glActiveTexture ( GL_TEXTURE1 );
		glBindTexture ( GL_TEXTURE_2D, gBuffer1 );
		glActiveTexture ( GL_TEXTURE2 );
		glBindTexture ( GL_TEXTURE_2D, gBuffer2 );

		glBindVertexArray ( emptyVAO );
		glDrawArrays ( GL_TRIANGLES, 0, 3 );

		glEnable ( GL_DEPTH_TEST );

		glUseProgram ( 0 );
	}

	glBindFramebuffer ( GL_FRAMEBUFFER, 0 );
}

void DeferredPassTestScene::Unload ()
{	 
}

void DeferredPassTestScene::Cleanup ()
{	 
}

void DeferredPassTestScene::LoadGBuffers () {
	glGenTextures ( 1, &gBuffer0 );
	glGenTextures ( 1, &gBuffer1 );
	glGenTextures ( 1, &gBuffer2 );
	glGenTextures ( 1, &depthTextureHandle );


	glBindTexture ( GL_TEXTURE_2D, gBuffer0 );
	glTexImage2D ( GL_TEXTURE_2D, 0, GL_RGBA16F, 1920, 1080, 0, GL_RGBA, GL_FLOAT, NULL );
	glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

	glBindTexture ( GL_TEXTURE_2D, gBuffer1 );
	glTexImage2D ( GL_TEXTURE_2D, 0, GL_RGBA16F, 1920, 1080, 0, GL_RGBA, GL_FLOAT, NULL );
	glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

	glBindTexture ( GL_TEXTURE_2D, gBuffer2 );
	glTexImage2D ( GL_TEXTURE_2D, 0, GL_RGBA16F, 1920, 1080, 0, GL_RGBA, GL_FLOAT, NULL );
	glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

	glBindTexture ( GL_TEXTURE_2D, depthTextureHandle );
	glTexImage2D ( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, 1920, 1080, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL );

	glGenFramebuffers ( 1, &gBufferFillFBO );
	glBindFramebuffer ( GL_FRAMEBUFFER, gBufferFillFBO );
	glFramebufferTexture2D ( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gBuffer0, 0 );
	glFramebufferTexture2D ( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gBuffer1, 0 );
	glFramebufferTexture2D ( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gBuffer2, 0 );

	//Depth buffer
	glFramebufferTexture2D ( GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTextureHandle, 0 );
	
	unsigned int attachments[ 3 ] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers ( 3, attachments );

	GLenum status = glCheckFramebufferStatus ( GL_FRAMEBUFFER );
	if ( status != GL_FRAMEBUFFER_COMPLETE ) {
		fprintf ( stderr, "Framebuffer Error. Status 0x%x\n", status );
	}

	glBindFramebuffer ( GL_FRAMEBUFFER, 0 );
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

			ImGui::DragFloat ( "metallic", &pSelectedObject->mMaterial.metallic, 0.01f, 0, 1 );
			ImGui::DragFloat ( "roughness", &pSelectedObject->mMaterial.roughness, 0.01f, 0, 1 );

			ImGui::DragFloat ( "emission", &pSelectedObject->mMaterial.emission, 0.01f, 0, 1);
		}

		ImGui::End ();

		ImGui::Begin ( "DebugRT", 0 );

		ImVec2 itemRectSize = ImGui::GetItemRectSize ();
		const float itemSizeScalar = itemRectSize.x / 1920.f;


		ImVec2 itemSize = { 1920 * itemSizeScalar, 1080 * itemSizeScalar };

		//ImGui::Image ( ( void* ) ( intptr_t ) gBuffer0, itemSize, {0, 1}, {1, 0});
		//ImGui::Image ( ( void* ) ( intptr_t ) gBuffer1, itemSize, { 0, 1 }, { 1, 0 } );
		//ImGui::Image ( ( void* ) ( intptr_t ) gBuffer2, itemSize, { 0, 1 }, { 1, 0 } );

		//ImGui::Image ( ( void* ) ( intptr_t ) depthTextureHandle, itemSize, { 0, 1 }, { 1, 0 } );
		ImGui::End ();

		{
			ImGui::Begin ( "Lights", 0 );
			ImGui::SeparatorText ( "Point" );
			{
				bool edited = false;
				PointLight& pointLight = sceneLights.pointLights[ 0 ];
				edited |= ImGui::ColorEdit3 ( "col", &pointLight.RGBColor.r );
				edited |= ImGui::SliderFloat3 ( "pos", &pointLight.position.x, -5.f, 5.f );
				edited |= ImGui::DragFloat ( "lumen", &pointLight.lumen, 0.01f, 1, 20000 );

				if( edited )
				{
					glBindBuffer ( GL_UNIFORM_BUFFER, sceneLightsUBO );
					glBufferSubData ( GL_UNIFORM_BUFFER, 0, sizeof ( ubSceneLights ), &sceneLights );
					glBindBuffer ( GL_UNIFORM_BUFFER, 0 );
				}
			}

			ImGui::SeparatorText ( "Point" );
		}


		ImGui::End ();
	};

	RegisterGUIUpdateCallback ( RenderGUIFunc );
}
