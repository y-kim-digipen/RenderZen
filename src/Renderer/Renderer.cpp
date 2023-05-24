#include "Renderer.h"

#include <cassert>
#include <iostream>
#include <string>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <fstream>
#include <glm/glm.hpp>
#include <sstream>
#include <utility>

#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "../Common/Mesh.h"

#include "../Tools/Input/Input.h"


Renderer gRenderer;

std::function<void ()> gGUIUpdateCallback;

void GLFWerrCallback ( int err, const char* desc ) {
	std::cerr << "GLFW Error: " << desc << std::endl;
}

unsigned Renderer::LoadVAO(VAOLoadDesc desc)
{
	unsigned VAO;
	glGenVertexArrays ( 1, &VAO );
	glBindVertexArray ( VAO );

	uint64_t offset = 0;
	for ( uint32_t i = 0; i < desc.numAttributes; ++i ) {
		auto& attribute = desc.mAttributes[ i ];

		uint32_t bufferType = attribute.mIsEBO ? GL_ELEMENT_ARRAY_BUFFER : GL_ARRAY_BUFFER;

		glBindBuffer ( bufferType, attribute.mBufferHandle );
		assert ( desc.mAttributes->pBuffer );

		if ( desc.mAttributes->pBuffer ) {
			glBufferData ( bufferType,
						   ( GLsizei ) attribute.mDataSize, attribute.pBuffer, GL_STATIC_DRAW );
		}

		if ( attribute.mIsEBO == false ) {
			glVertexAttribPointer ( i, ( GLint ) attribute.mSize, attribute.mType, GL_FALSE, ( GLsizei ) attribute.mSize * attribute.mTypeSize, 0/*(void*)offset*/ );
			offset += attribute.mDataSize;
			glEnableVertexAttribArray ( i );
		}
	}

	glBindVertexArray ( 0 );
	return VAO;
}

unsigned Renderer::MakeBuffer()
{
	unsigned bufferHandle;
	glGenBuffers ( 1, &bufferHandle );
	return bufferHandle;
}

bool Renderer::LoadShader( ShaderLoadDesc desc )
{
	GLint  success;
	char infoLog[ 512 ];

	GLint shaderProgram = glCreateProgram ();

	//GLint compiledShaderHandles[ 2 ]{};
	//Compile vertex shader
	{
		const char* name = desc.vertexShaderName;

		GLint shaderHandle = glCreateShader ( GL_VERTEX_SHADER );

		std::ifstream vertexShaderFile ( name, std::ifstream::in );

		if( !vertexShaderFile )
		{
			assert ( true );
			return false;
		}

		std::stringstream buffer;
		buffer << vertexShaderFile.rdbuf ();
		vertexShaderFile.close ();

		std::string shaderSource = buffer.str ();
		const char* shaderSourceStr = shaderSource.c_str ();

		glShaderSource ( shaderHandle, 1, &shaderSourceStr, NULL );
		glCompileShader ( shaderHandle );

		glGetShaderiv ( shaderHandle, GL_COMPILE_STATUS, &success );
		if ( !success ) {
			glGetShaderInfoLog ( shaderHandle, 512, NULL, infoLog );
			std::cout << "ERROR::SHADER::VERT::COMPILATION_FAILED\n" << infoLog << std::endl;

			assert ( false );
			return false;

			glDeleteProgram ( shaderProgram );
			return false;
		}
		glAttachShader ( shaderProgram, shaderHandle );
		//glDeleteShader ( shaderHandle );
	}

	//Compile fragment shader
	{
		const char* name = desc.fragShaderName;

		GLint shaderHandle = glCreateShader ( GL_FRAGMENT_SHADER );

		std::ifstream vertexShaderFile ( name, std::ifstream::in );

		if ( !vertexShaderFile ) {
			//todo
			std::cout << "ERROR::SHADER::FRAG::FILE_DOES_NOT_EXIST\n" << name << std::endl;
			assert ( false );
			return false;
		}

		std::stringstream buffer;
		buffer << vertexShaderFile.rdbuf ();
		vertexShaderFile.close ();

		std::string shaderSource = buffer.str ();
		const char* shaderSourceStr = shaderSource.c_str ();

		glShaderSource ( shaderHandle, 1, &shaderSourceStr, NULL );
		glCompileShader ( shaderHandle );

		glGetShaderiv ( shaderHandle, GL_COMPILE_STATUS, &success );
		if ( !success ) {
			glGetShaderInfoLog ( shaderHandle, 512, NULL, infoLog );
			std::cout << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;

			assert ( true );
			return false;
		}
		glAttachShader ( shaderProgram, shaderHandle );
		//glDeleteShader ( shaderHandle );
	}

	glLinkProgram ( shaderProgram );

	glGetProgramiv ( shaderProgram, GL_LINK_STATUS, &success );
	if ( !success ) {
		glGetProgramInfoLog ( shaderProgram, 512, NULL, infoLog );
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}

	pShaderPrograms.push_back ( new ShaderProgram { shaderProgram } );

	return true;
}

bool Renderer::Init()
{
	if ( !GLFWInit () ) return false;
	if ( !gladLoadGLLoader ( ( GLADloadproc ) glfwGetProcAddress ) ) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return false;
	}

	return true;
}

bool Renderer::Load()
{
	//glEnable ( GL_MULTISAMPLE );

	//Load GUI
	const char* glsl_version = "#version 450";

	IMGUI_CHECKVERSION ();
	ImGui::CreateContext ();
	ImGuiIO& io = ImGui::GetIO ();
	
	ImGui_ImplGlfw_InitForOpenGL ( pGLFWwindow, true );
	ImGui_ImplOpenGL3_Init ( glsl_version );


	ShaderLoadDesc shaderLoadDesc { "assets/shaders/default.vert", "assets/shaders/default.frag" };
	bool shaderLoadRes = LoadShader ( shaderLoadDesc );

	shaderLoadDesc =  { "assets/shaders/GBufferFill.vert", "assets/shaders/GBufferFill.frag" };
	shaderLoadRes &= LoadShader ( shaderLoadDesc );

	shaderLoadDesc = { "assets/shaders/DeferredQuadPass.vert", "assets/shaders/BRDFLighting.frag" };
	shaderLoadRes &= LoadShader ( shaderLoadDesc );

	return shaderLoadRes;
}

void Renderer::Update()
{
	ImGui_ImplOpenGL3_NewFrame ();
	ImGui_ImplGlfw_NewFrame ();
	ImGui::NewFrame ();
	ImGui::ShowDemoWindow ( &mGuiControl.showDemoWindow );
	if ( gGUIUpdateCallback ) ( gGUIUpdateCallback )( );
}

void Renderer::Draw()
{
	//glfwGetFramebufferSize ( pGLFWwindow, &gRendererState.mScreenSize.x, &gRendererState.mScreenSize.y );
	//glViewport ( 0, 0, gRendererState.mScreenSize.x, gRendererState.mScreenSize.y );
	ImGui::Render ();
	ImGui_ImplOpenGL3_RenderDrawData ( ImGui::GetDrawData () );
	glfwSwapBuffers ( pGLFWwindow );
}

void Renderer::Unload()
{

}

bool Renderer::GLFWInit()
{
	if ( !glfwInit () ) return false;

	glfwWindowHint ( GLFW_CONTEXT_VERSION_MAJOR, 4 );
	glfwWindowHint ( GLFW_CONTEXT_VERSION_MINOR, 5 );

	glfwWindowHint ( GLFW_OPENGL_DEBUG_CONTEXT, true );

	glfwWindowHint ( GLFW_SAMPLES, 4 );

	//Create GLFW window
	static std::string appTitle = "RenderZen v0.0";

	GLFWwindow* pWindow
		= glfwCreateWindow ( 1920, 1080, appTitle.c_str (), NULL, NULL );
	if ( !pWindow ) return false;

	glfwSetErrorCallback ( GLFWerrCallback );
	//Set input callbacks
	glfwSetKeyCallback ( pWindow, Input::Input::GLFW_KeyboardCallback );
	glfwSetMouseButtonCallback ( pWindow, Input::Input::GLFW_MouseButtonCallback );
	glfwSetCursorPosCallback ( pWindow, Input::Input::GLFW_CursorCallback );

	glfwMakeContextCurrent ( pWindow );

	pGLFWwindow = pWindow;

	return pGLFWwindow;
}

VAOLoadDesc MakeVAOLoadDesc(Mesh* pMesh)
{
	VAOLoadDesc vaoLoadDesc {};
	unsigned numVertices = ( unsigned ) pMesh->mPositions.size ();
	vaoLoadDesc.mAttributes[ 0 ].mIsEBO = false;
	vaoLoadDesc.mAttributes[ 0 ].mSize = 3;
	vaoLoadDesc.mAttributes[ 0 ].mType = GL_FLOAT;
	vaoLoadDesc.mAttributes[ 0 ].mTypeSize = sizeof ( float );
	vaoLoadDesc.mAttributes[ 0 ].mBufferHandle = gRenderer.MakeBuffer ();
	vaoLoadDesc.mAttributes[ 0 ].mDataSize = numVertices * sizeof ( glm::vec3 );
	vaoLoadDesc.mAttributes[ 0 ].pBuffer = pMesh->mPositions.data ();

	vaoLoadDesc.mAttributes[ 1 ].mIsEBO = false;
	vaoLoadDesc.mAttributes[ 1 ].mSize = 3;
	vaoLoadDesc.mAttributes[ 1 ].mType = GL_FLOAT;
	vaoLoadDesc.mAttributes[ 1 ].mTypeSize = sizeof ( float );
	vaoLoadDesc.mAttributes[ 1 ].mBufferHandle = gRenderer.MakeBuffer ();
	vaoLoadDesc.mAttributes[ 1 ].mDataSize = numVertices * sizeof ( glm::vec3 );
	vaoLoadDesc.mAttributes[ 1 ].pBuffer = pMesh->mNormals.data ();

	vaoLoadDesc.mAttributes[ 2 ].mIsEBO = false;
	vaoLoadDesc.mAttributes[ 2 ].mSize = 2;
	vaoLoadDesc.mAttributes[ 2 ].mType = GL_FLOAT;
	vaoLoadDesc.mAttributes[ 2 ].mTypeSize = sizeof ( float );
	vaoLoadDesc.mAttributes[ 2 ].mBufferHandle = gRenderer.MakeBuffer ();
	vaoLoadDesc.mAttributes[ 2 ].mDataSize = numVertices * sizeof ( glm::vec2 );
	vaoLoadDesc.mAttributes[ 2 ].pBuffer = pMesh->mUVs.data ();

	vaoLoadDesc.mAttributes[ 3 ].mIsEBO = true;
	vaoLoadDesc.mAttributes[ 3 ].mBufferHandle = gRenderer.MakeBuffer ();
	vaoLoadDesc.mAttributes[ 3 ].mDataSize = ( unsigned ) pMesh->mIndices.size () * sizeof ( uint32_t );
	vaoLoadDesc.mAttributes[ 3 ].pBuffer = pMesh->mIndices.data ();

	vaoLoadDesc.numAttributes = 4;

	return vaoLoadDesc;
}

void RegisterGUIUpdateCallback(std::function<void()> callback)
{
	gGUIUpdateCallback = std::move(callback);
}
