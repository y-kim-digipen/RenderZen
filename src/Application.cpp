#include "Application.h"

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <map>

#include <GLAD/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
//#include "IRenderer.h"

#include "imnodes.h"
#include "../Tools/AssetLoader/src/IAssetLoader.h"
#include "../ThirdParty/imgui-1.89.3/example/node_editor.h"

#include "ShaderUniformManager.h"
#include "backends/Defsr.h"
#include "backends/IType.h"

#define APP_NAME "RenderZen"
#define MAJOR_VERSION 0
#define MINOR_VERSION 0

#define MAX_UNIFORM_NAME_LEN 30

GLFWwindow* pGLFWwindow{ nullptr };

glm::ivec2 gAppScreenSize{-1};


Mesh* pMesh;
Mesh* pCubeMesh;

uint32_t gSelectedShaderIndex = 0;
uint32_t numShader = 0;
std::vector<uint32_t> gShader;
std::vector<const char*> gShaderNames;

ShaderUniformManager<> gShaderUniformManger;

//For Testing

float vertices[] = {
	 0.5f,  0.5f, 0.0f,  // top right
	 0.5f, -0.5f, 0.0f,  // bottom right
	-0.5f, -0.5f, 0.0f,  // bottom left
	-0.5f,  0.5f, 0.0f   // top left 
};
unsigned int indices[] = {  // note that we start from 0!
	0, 1, 3,   // first triangle
	1, 2, 3    // second triangle
};

namespace IRenderer {
	struct VertexAttributes
	{
		struct
		{
			uint32_t	mBufferHandle;
			uint32_t	mDataSize;
			void*		pBuffer;
			bool		mIsEBO;
			uint32_t	mType;
			uint32_t	mSize;
			uint32_t	mTypeSize;
		} mAttributes[MAX_VERTEXATTRIB_LAYOUTS];

		uint32_t numAttributes;
	};

	enum SHADER_TYPE
	{
		SHADER_TYPE_VERTEX_SHADER = GL_VERTEX_SHADER,
		SHADER_TYPE_FRAGMENT_SHADER = GL_FRAGMENT_SHADER,
	};

	struct ShaderInitDescriptorSets
	{
		struct
		{
			const char* mPath;
			SHADER_TYPE mType;
		} mShaders[5];
		uint32_t numShaders;
	};

	uint32_t InitShader(ShaderInitDescriptorSets shaderDescriptorSets)
	{
		int32_t success;
		char infoLog[512];

		uint32_t shaderProgramHandle = glCreateProgram();
		uint32_t shaderHandles[5];
		//Compile Shader
		for(uint32_t i = 0; i < shaderDescriptorSets.numShaders; ++i)
		{
			auto& shaderDesc = shaderDescriptorSets.mShaders[i];
			uint32_t shaderHandle = glCreateShader(shaderDesc.mType);
			//todo read from file

			std::ifstream vertexShaderFile(shaderDesc.mPath, std::ifstream::in);
			if (!vertexShaderFile)
			{
				std::cout << "ERROR::SHADER::FILE_DOES_NOT_EXIST\n" << shaderDesc.mPath << std::endl;

				for (uint32_t j = 0; j <= i; ++j)
				{
					glDeleteShader(shaderHandles[j]);
				}

				glDeleteProgram(shaderProgramHandle);
				return false;
			}

			std::stringstream buffer;
			buffer << vertexShaderFile.rdbuf();
			vertexShaderFile.close();

			std::string shaderSource = buffer.str();
			const char* shaderSourceStr = shaderSource.c_str();

			glShaderSource(shaderHandle, 1, &shaderSourceStr, NULL);
			glCompileShader(shaderHandle);

			glGetShaderiv(shaderHandle, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(shaderHandle, 512, NULL, infoLog);
				std::cout << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;

				for(uint32_t j = 0 ; j <= i; ++j)
				{
					glDeleteShader(shaderHandles[j]);
				}

				glDeleteProgram(shaderProgramHandle);
				return -1;
			}
			glAttachShader(shaderProgramHandle, shaderHandle);
		}

		glLinkProgram(shaderProgramHandle);

		for (uint32_t i = 0; i < shaderDescriptorSets.numShaders; ++i)
		{
			glDeleteShader(shaderHandles[i]);
		}

		glGetProgramiv(shaderProgramHandle, GL_LINK_STATUS, &success);
		if (!success) {
			glGetProgramInfoLog(shaderProgramHandle, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::LINKING_FAILED\n" << infoLog << std::endl;
		}

		return shaderProgramHandle;
	}

	bool InitRenderer()
	{
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		{
			//std::cout << "Failed to initialize GLAD" << std::endl;
			return false;
		}

		glEnable(GL_MULTISAMPLE);
		return true;
	}

	void SetClearColor(glm::vec4 color)
	{
		glClearColor(color.r, color.g, color.b, color.a);
	}

	uint32_t LoadVAO(VertexAttributes attributes)
	{
		uint32_t VAO;
		glGenVertexArrays(1, &VAO);
		// 1. bind Vertex Array Object
		glBindVertexArray(VAO);

		uint64_t offset = 0;
		for (uint32_t i = 0; i < attributes.numAttributes; ++i)
		{
			auto& attribute = attributes.mAttributes[i];

			uint32_t bufferType = attribute.mIsEBO ? GL_ELEMENT_ARRAY_BUFFER : GL_ARRAY_BUFFER;

			glBindBuffer(bufferType, attribute.mBufferHandle);
			assert(attributes.mAttributes->pBuffer);

			if (attributes.mAttributes->pBuffer)
			{
				glBufferData(bufferType,
					(GLsizei)attribute.mDataSize, attribute.pBuffer, GL_STATIC_DRAW);
			}

			if(attribute.mIsEBO == false)
			{
				glVertexAttribPointer(i, (GLint)attribute.mSize, attribute.mType, GL_FALSE, (GLsizei)attribute.mSize * attribute.mTypeSize, 0/*(void*)offset*/);
				offset += attribute.mDataSize;
				glEnableVertexAttribArray(i);
			}
		}

		glBindVertexArray(0);
		return VAO;
	}

	uint32_t GenBuffer()
	{
		uint32_t bufferHandle = 0;
		glGenBuffers(1, &bufferHandle);
		return bufferHandle;
	}

	//void TrySetUniform(uint32_t shaderIndex, int32_t gpuBindLoc, DataTypeFlag flag)
	//{
	//	bool isFloat = true;
	//	bool isVec = flag & DataTypeFlags_VEC_TYPE;
	//	bool isMat = flag & DataTypeFlags_MAT_TYPE;
	//	//int32_t unfiromIndex = gShaderUniformManger.[ shaderIndex ].mDataOffsets[ gpuBindLoc ].mGPUIndex;
	//	uint32_t dim = 1;
	//	if ( isVec ) {
	//		for ( uint32_t d = 2; d <= 4; ++d ) {
	//			if ( flag & ( DataTypeFlags_VEC_TYPE << ( d - 1 ) ) ) {
	//				dim = d;
	//			}
	//		}
	//	}
	//	if ( isMat ) {
	//		for ( uint32_t d = 2; d <= 4; ++d ) {
	//			if ( flag & ( DataTypeFlags_MAT_TYPE << ( d - 1 ) ) ) {
	//				dim = d;
	//			}
	//		}
	//	}
	//	void* buffer = ShaderUniformManager::GetDataFromGPULoc ( shaderIndex, gpuBindLoc );
	//	if(isFloat )
	//	{
	//		if(isVec )
	//		{
	//			switch ( dim )
	//			{
	//				case 2:	glUniform2fv ( unfiromIndex, 1, ( const GLfloat* ) buffer ); break;
	//				case 3:	glUniform3fv ( unfiromIndex, 1, ( const GLfloat* ) buffer ); break;
	//				case 4:	glUniform4fv ( unfiromIndex, 1, ( const GLfloat* ) buffer ); break;
	//				default: assert ( true );
	//			}
	//		}
	//		else if(isMat )
	//		{
	//			auto mat = *( glm::mat4* ) buffer;
	//			switch ( dim ) {
	//				case 2:	glUniformMatrix2fv ( unfiromIndex, 1, false, ( const GLfloat* ) buffer ); break;
	//				case 3:	glUniformMatrix3fv ( unfiromIndex, 1, false, ( const GLfloat* ) buffer ); break;
	//				case 4:	glUniformMatrix4fv ( unfiromIndex, 1, false, ( const GLfloat* ) buffer ); break;
	//				default: assert ( true );
	//			}
	//		}
	//		else
	//		{

	//			uint32_t di = glGetUniformLocation ( gShader[ shaderIndex ], "toWorldMat" );
	//			glUniform1fv ( unfiromIndex, 1, ( const GLfloat* ) buffer );
	//		}
	//	}
	//	else assert ( true );
	//	ShaderUniformManager::mDesc[ shaderIndex ].mDataOffsets[ gpuBindLoc ].mIsDirty = false;

	//}

#define MAX_RT 10
	struct FrameBufferLoadDesc
	{
		struct
		{
			uint32_t mTextureHandle;
			glm::uvec2 mSize;
			uint32_t mInternalFormat;
			uint32_t mFormat;
			uint32_t mDataFormat;
			GLint mMinSamplerState;
			GLint mMagSamplerState;
			GLint mAttachmentSlot;
		} mRenderTargets[MAX_RT];

		uint32_t numRenderTargets;
	};

	uint32_t GenFrameBuffer(FrameBufferLoadDesc desc)
	{
		uint32_t frameBuffer;
		glGenFramebuffers(1, &frameBuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);


		for(uint32_t i = 0; i < desc.numRenderTargets; ++i)
		{
			const auto& renderTargets = desc.mRenderTargets[i];
			glBindTexture(GL_TEXTURE_2D, renderTargets.mTextureHandle);
			glTexImage2D(GL_TEXTURE_2D, 0, renderTargets.mInternalFormat, 
				renderTargets.mSize.x, renderTargets.mSize.y, 0, renderTargets.mFormat, renderTargets.mDataFormat, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, renderTargets.mMinSamplerState);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, renderTargets.mMagSamplerState);
			glFramebufferTexture2D(GL_FRAMEBUFFER, renderTargets.mAttachmentSlot, GL_TEXTURE_2D, renderTargets.mTextureHandle, 0);
		}


		//// - tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
		//unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
		//glDrawBuffers(3, attachments);

		return frameBuffer;
	}

	struct alignas(sizeof(glm::mat4)) cbCamera
	{
		glm::mat4 viewMat;
		glm::mat4 projectionMat;
		glm::vec3 camPos;
	};
}

namespace
{
	void GLFWerrCallback(int err, const char* desc)
	{
		std::cerr << "GLFW Error: " << desc << std::endl;
	}

	static void GLFWkeyboardCallback(GLFWwindow* pWindow, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
			glfwSetWindowShouldClose(pWindow, GLFW_TRUE);
	}

	GLFWwindow* GLFWinit()
	{
		if (!glfwInit()) return nullptr;

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);

		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);

		glfwWindowHint(GLFW_SAMPLES, 4);

		//Create GLFW window
		static std::string appTitle = std::string(APP_NAME) + " v"
			+ std::to_string(MAJOR_VERSION) + '.' + std::to_string(MINOR_VERSION);

		GLFWwindow* pWindow
			= glfwCreateWindow(1920, 1080, appTitle.c_str(), NULL, NULL);
		if (!pWindow) return nullptr;

		glfwSetErrorCallback(GLFWerrCallback);
		//Set input callbacks
		glfwSetKeyCallback(pWindow, GLFWkeyboardCallback);

		glfwMakeContextCurrent(pWindow);
		
		return pWindow;
	}

	void imgui_easy_theming()
	{
		ImGuiStyle& style = ImGui::GetStyle();
		style.Colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
		style.Colors[ImGuiCol_WindowBg] = ImVec4(0.13f, 0.14f, 0.15f, 1.00f);
		style.Colors[ImGuiCol_ChildBg] = ImVec4(0.13f, 0.14f, 0.15f, 1.00f);
		style.Colors[ImGuiCol_PopupBg] = ImVec4(0.13f, 0.14f, 0.15f, 1.00f);
		style.Colors[ImGuiCol_Border] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
		style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		style.Colors[ImGuiCol_FrameBg] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
		style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.38f, 0.38f, 0.38f, 1.00f);
		style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.67f, 0.67f, 0.67f, 0.39f);
		style.Colors[ImGuiCol_TitleBg] = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
		style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
		style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
		style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
		style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
		style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
		style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
		style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
		style.Colors[ImGuiCol_CheckMark] = ImVec4(0.11f, 0.64f, 0.92f, 1.00f);
		style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.11f, 0.64f, 0.92f, 1.00f);
		style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.08f, 0.50f, 0.72f, 1.00f);
		style.Colors[ImGuiCol_Button] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
		style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.38f, 0.38f, 0.38f, 1.00f);
		style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.67f, 0.67f, 0.67f, 0.39f);
		style.Colors[ImGuiCol_Header] = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
		style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
		style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.67f, 0.67f, 0.67f, 0.39f);
		style.Colors[ImGuiCol_Separator] = style.Colors[ImGuiCol_Border];
		style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.41f, 0.42f, 0.44f, 1.00f);
		style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
		style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.29f, 0.30f, 0.31f, 0.67f);
		style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
		style.Colors[ImGuiCol_Tab] = ImVec4(0.08f, 0.08f, 0.09f, 0.83f);
		style.Colors[ImGuiCol_TabHovered] = ImVec4(0.33f, 0.34f, 0.36f, 0.83f);
		style.Colors[ImGuiCol_TabActive] = ImVec4(0.23f, 0.23f, 0.24f, 1.00f);
		style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
		style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.13f, 0.14f, 0.15f, 1.00f);
		//style.Colors[ImGuiCol_DockingPreview] = ImVec4(0.26f, 0.59f, 0.98f, 0.70f);
		//style.Colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
		style.Colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
		style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
		style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
		style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
		style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
		style.Colors[ImGuiCol_DragDropTarget] = ImVec4(0.11f, 0.64f, 0.92f, 1.00f);
		style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
		style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
		style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
		style.GrabRounding = style.FrameRounding = 2.3f;
	}

	void InitGUI()
	{
		const char* glsl_version = "#version 450";
		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		
		//Docking options
		//io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		
		// Setup Platform/Renderer bindings
		ImGui_ImplGlfw_InitForOpenGL ( pGLFWwindow, true );
		ImGui_ImplOpenGL3_Init ( glsl_version );
		// Setup Dear ImGui style
		imgui_easy_theming ();
		//ImGui::StyleColorsDark();
		ImNodes::CreateContext ();
	}

	void LoadShader(uint32_t shaderHandle, const char* shaderName)
	{
		std::cout << "VERB::SHADER::LOAD::Start loading shader: " << shaderName << std::endl;
		int32_t count;
		glGetProgramiv ( shaderHandle, GL_ACTIVE_UNIFORMS, &count );

		std::cout << "VERB::SHADER::LOAD::Detecting uniforms from shader: " << shaderName << std::endl;
		gShaderUniformManger.LoadBuffer ( shaderHandle );


		//[numShader] .numActiveBindings = 0;
		//for ( int32_t index = 0; index < MAX_BINDING; ++index )
		//{
		//	ShaderUniformManager::mDesc[ numShader ].mGPUBindLocations[ index ] = -1;
		//}
		////glUseProgram ( shaderHandle );
		//uint32_t offset = 0;
		//for ( int32_t uniformIndex = 0, cpuBindLocation = 0; uniformIndex < count; ++uniformIndex )
		//{
		//	glGetActiveUniform ( shaderHandle, uniformIndex,
		//						MAX_UNIFORM_NAME_LEN, &length, 
		//						&size, &glDataType, name );

		//	int32_t gpuBindLocation = glGetUniformLocation ( shaderHandle, name );
		//	if ( gpuBindLocation >= 0 ) {
		//		//const auto& dataTypeDescLookupTable = GetDataTypeDescLookupTable ();
		//		DataTypeDesc dataTypeDesc = GetDataTypeDesc( glDataType );

		//		const char* dataTypeStr =dataTypeDesc.mName;
		//		std::cout << name << ": " << dataTypeStr << ":" << size << ":" << uniformIndex << std::endl;

		//		const uint32_t dataSize = dataTypeDesc.mSize;

		//		auto& desc = ShaderUniformManager::mDesc[ numShader ];
		//		desc.mGPUBindLocations[ cpuBindLocation++ ] = uniformIndex;
		//		bool emplaceRes
		//			= desc.mDataOffsets.emplace( uniformIndex, DataTypeDesc { offset, glDataType,true, false, gpuBindLocation } ).second;

		//		//Should not have same gpu binding in one shader buffer
		//		assert ( emplaceRes );

		//		offset += dataSize;
		//		desc.numActiveBindings++;
		//	}
		//}

		//std::cout << "VERB::SHADER::LOAD::Detecting uniforms blocks from shader: " << shaderName << std::endl;
		//glGetProgramiv(shaderHandle, GL_ACTIVE_UNIFORM_BLOCKS, &count);

		//for (int32_t uniformBlockIndex = 0; uniformBlockIndex < count; ++uniformBlockIndex)
		//{
		//	glGetActiveUniformBlockName(shaderHandle, uniformBlockIndex,
		//		MAX_UNIFORM_NAME_LEN, &length, name);

		//	std::cout << name << std::endl;
		//}


		gShader.push_back(shaderHandle);
		gShaderNames.push_back(shaderName);

		numShader++;
	}
}

void LoadGeometries()
{
	pMesh = new Mesh;
	CPULoadMesh("stanford-bunny.fbx", pMesh);
}

bool Application::Load()
{
	//Load Assets
	LoadGeometries();

	glfwSwapBuffers(pGLFWwindow);
	glfwSwapInterval(1);
	GLenum err = glGetError();
	return true;
}

bool Application::Init()
{
	pGLFWwindow = GLFWinit();
	if (!pGLFWwindow) return false;
	if (!IRenderer::InitRenderer()) return false;
	InitGUI();

	//Load asset manager interfaces
	SetMeshDirectory("assets/meshes");

	return true;
}

void Application::Update () {
	IRenderer::VertexAttributes attributes {};
	attributes.mAttributes[ 0 ].mIsEBO = false;
	attributes.mAttributes[ 0 ].mSize = 3;
	attributes.mAttributes[ 0 ].mType = GL_FLOAT;
	attributes.mAttributes[ 0 ].mTypeSize = sizeof ( float );
	attributes.mAttributes[ 0 ].mBufferHandle = IRenderer::GenBuffer ();
	attributes.mAttributes[ 0 ].mDataSize = pMesh->mData.mNumVertices * sizeof ( glm::vec3 );
	attributes.mAttributes[ 0 ].pBuffer = pMesh->mPositions.data ();

	attributes.mAttributes[ 1 ].mIsEBO = false;
	attributes.mAttributes[ 1 ].mSize = 3;
	attributes.mAttributes[ 1 ].mType = GL_FLOAT;
	attributes.mAttributes[ 1 ].mTypeSize = sizeof ( float );
	attributes.mAttributes[ 1 ].mBufferHandle = IRenderer::GenBuffer ();
	attributes.mAttributes[ 1 ].mDataSize = pMesh->mData.mNumVertices * sizeof ( glm::vec3 );
	attributes.mAttributes[ 1 ].pBuffer = pMesh->mNormals.data ();

	attributes.mAttributes[ 2 ].mIsEBO = false;
	attributes.mAttributes[ 2 ].mSize = 2;
	attributes.mAttributes[ 2 ].mType = GL_FLOAT;
	attributes.mAttributes[ 2 ].mTypeSize = sizeof ( float );
	attributes.mAttributes[ 2 ].mBufferHandle = IRenderer::GenBuffer ();
	attributes.mAttributes[ 2 ].mDataSize = pMesh->mData.mNumVertices * sizeof ( glm::vec2 );
	attributes.mAttributes[ 2 ].pBuffer = pMesh->mUVs.data ();

	attributes.mAttributes[ 3 ].mIsEBO = true;
	attributes.mAttributes[ 3 ].mBufferHandle = IRenderer::GenBuffer ();
	attributes.mAttributes[ 3 ].mDataSize = pMesh->mData.mNumIndices * sizeof ( uint32_t );
	attributes.mAttributes[ 3 ].pBuffer = pMesh->mIndices.data ();

	attributes.numAttributes = 4;

	uint32_t VAO = IRenderer::LoadVAO ( attributes );

	static bool isInitialized = false;

	//Add shader
	IRenderer::ShaderInitDescriptorSets shaderDesc {};
	shaderDesc.numShaders = 2;
	shaderDesc.mShaders[ 0 ].mType = IRenderer::SHADER_TYPE_VERTEX_SHADER;
	shaderDesc.mShaders[ 0 ].mPath = "assets/shaders/default.vert";

	shaderDesc.mShaders[ 1 ].mType = IRenderer::SHADER_TYPE_FRAGMENT_SHADER;
	shaderDesc.mShaders[ 1 ].mPath = "assets/shaders/default.frag";

	uint32_t defaultShaderHandle = IRenderer::InitShader ( shaderDesc );


	shaderDesc.numShaders = 2;
	shaderDesc.mShaders[ 0 ].mType = IRenderer::SHADER_TYPE_VERTEX_SHADER;
	shaderDesc.mShaders[ 0 ].mPath = "assets/shaders/emissive.vert";

	shaderDesc.mShaders[ 1 ].mType = IRenderer::SHADER_TYPE_FRAGMENT_SHADER;
	shaderDesc.mShaders[ 1 ].mPath = "assets/shaders/emissive.frag";

	uint32_t emissionShaderHandle = IRenderer::InitShader ( shaderDesc );

	LoadShader ( defaultShaderHandle, "DefaultShader" );
	LoadShader ( emissionShaderHandle, "Emissive" );

		//Init commons
	for ( uint32_t shaderIndex = 0; shaderIndex < numShader; ++shaderIndex ) {
		uint32_t shaderProgramHandle = gShader[ shaderIndex ];
		unsigned int uboIndex = glGetUniformBlockIndex ( shaderProgramHandle, "cbCamera" );
		glUniformBlockBinding ( shaderProgramHandle, uboIndex, 0 );

		unsigned int uboMatrices;
		glGenBuffers ( 1, &uboMatrices );

		glBindBuffer ( GL_UNIFORM_BUFFER, uboMatrices );
		glBufferData ( GL_UNIFORM_BUFFER, sizeof ( IRenderer::cbCamera ), NULL, GL_STATIC_DRAW );
		glBindBuffer ( GL_UNIFORM_BUFFER, 0 );

		glBindBufferRange ( GL_UNIFORM_BUFFER, 0, uboMatrices, 0, sizeof ( IRenderer::cbCamera ) );

		glm::vec3 cameraPos = glm::vec3 ( 0.0f, 0.0f, 30.0f );
		glm::vec3 cameraFront = glm::vec3 ( 0.0f, 0.0f, -1.0f );
		glm::vec3 cameraUp = glm::vec3 ( 0.0f, 1.0f, 0.0f );
		IRenderer::cbCamera camera {};
		camera.viewMat = glm::lookAt ( cameraPos, cameraPos + cameraFront, cameraUp );
		camera.projectionMat = glm::perspective ( glm::radians ( 45.0f ), ( float ) 1920 / ( float ) 1080, 0.1f, 100.0f );
		camera.camPos = { 0, 0, 3 };

		glBindBuffer ( GL_UNIFORM_BUFFER, uboMatrices );
		glBufferSubData ( GL_UNIFORM_BUFFER, 0, sizeof ( IRenderer::cbCamera ), &camera );
		glBindBuffer ( GL_UNIFORM_BUFFER, 0 );
	}

	//todo move other
	glEnable ( GL_DEPTH_TEST );


	while ( !glfwWindowShouldClose ( pGLFWwindow ) ) {
		glfwPollEvents ();
		glClearColor ( 0.2f, 0.3f, 0.3f, 1.0f );
		glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
		// feed inputs to dear imgui, start new frame
		ImGui_ImplOpenGL3_NewFrame ();
		ImGui_ImplGlfw_NewFrame ();
		ImGui::NewFrame ();

		if ( !isInitialized ) {
			example::NodeEditorInitialize ();
			isInitialized = true;
		}

		float time = ( float ) glfwGetTime ();

		glPushDebugGroup ( GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Hello Quad..?" );

		// 4. draw the object
		uint32_t shaderProgramHandle = gShader[ gSelectedShaderIndex ];
		glUseProgram ( shaderProgramHandle );

		static bool doRotate = false;

		//auto& desc = ShaderUniformManager::mDesc[ gSelectedShaderIndex ];
		//for(uint32_t uniformCpuBindIndex = 0; uniformCpuBindIndex < desc.numActiveBindings; ++uniformCpuBindIndex )
		//{
		//	int32_t gpuBindIndex = desc.mGPUBindLocations[ uniformCpuBindIndex ];
		//	IRenderer::TrySetUniform ( gSelectedShaderIndex, gpuBindIndex, 
		//	IRenderer::gSupportRawDataTypeFlagTable[desc.mDataOffsets[gpuBindIndex].glType] );
		//}

		/*int32_t vertexColorLocation = glGetUniformLocation ( shaderProgramHandle, "toWorldMat" );
		glm::mat4 identity { 1 };
		identity = glm::scale ( identity, glm::vec3 ( 0.001f ) );
		identity = glm::rotate ( identity, doRotate ? time : glm::pi<float> () * 0.25f, glm::vec3 ( 1, 1, 0 ) );
		glUniformMatrix4fv ( vertexColorLocation, 1, false, &identity[ 0 ][ 0 ] );*/


		glBindVertexArray ( VAO );
		glPolygonMode ( GL_BACK, GL_FILL );
		glEnable ( GL_DEPTH_TEST );
		glDrawElements ( GL_TRIANGLES, ( GLsizei ) pMesh->mData.mNumIndices, GL_UNSIGNED_INT, 0 );
		glPopDebugGroup ();

		// render your GUI
		ImGui::Begin ( "Demo window" );
		if ( ImGui::Button ( "Rotate!" ) ) {
			doRotate = !doRotate;
		}

		if ( ImGui::CollapsingHeader ( "Supported Types" ) ) {
			static bool showAsHex = false;
			ImGuiTableFlags flag = ImGuiTableFlags_ScrollY | ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableColumnFlags_WidthFixed | ImGuiTableFlags_Hideable;
			
			if ( ImGui::BeginTable ( "SupportedType", 3, flag, {0, ImGui::GetWindowHeight () * 0.15f }) ) {
				ImGui::TableNextColumn ();
				ImGui::Text ( "%s", "Type" );
				ImGui::TableNextColumn ();
				ImGui::Text ( "%s", "Size" );
				ImGui::TableNextColumn ();
				ImGui::Text ( "%s", "ApiID" );
				ImGui::SameLine ();
				static const char* cStrViewModes [] { "Dec", "Hex" };
				if(ImGui::BeginCombo( "##IDviewMode", cStrViewModes[showAsHex]) )
				{
					for(uint32_t i = 0; i < 2; ++i )
					{
						if ( ImGui::Selectable ( cStrViewModes[ i ], ( bool ) i == showAsHex ) )
						{
							showAsHex = (bool)i;
						}
					}
					ImGui::EndCombo ();
				}

				for ( const auto& type : GetDataTypeDescLookupTable () ) {
					uint32_t apiID = type.first;
					const char* nameCStr = type.second.mName;
					uint32_t size = type.second.mSize;

					ImGui::TableNextRow ();
					ImGui::TableNextColumn ();
					ImGui::Text ( "%s", nameCStr );
					ImGui::TableNextColumn ();
					ImGui::Text ( "%d", size );
					ImGui::TableNextColumn ();
					if( showAsHex )
					{
						std::stringstream ss;
						ss << std::hex << apiID;
						ImGui::Text ( "0x%s", ss.str ().c_str () );
					}
					else
					{
						ImGui::Text ( "0d%d", apiID );
					}
				}
				ImGui::EndTable ();
			}
		}

	if ( ImGui::BeginCombo ( "Shader", gShaderNames[ gSelectedShaderIndex ] ) ) {
		for ( uint32_t shaderIndex = 0; shaderIndex < numShader; ++shaderIndex ) {
			if ( ImGui::Selectable ( gShaderNames[ shaderIndex ], shaderIndex == gSelectedShaderIndex ) ) {
				gSelectedShaderIndex = shaderIndex;
			}
		}
		ImGui::EndCombo ();
	}

	if ( ImGui::CollapsingHeader ( "uniforms" ) ) {
		uint32_t shaderProgramHandle = gShader[ gSelectedShaderIndex ];


		auto& uniformAccessDesc = gShaderUniformManger.GetUniformAccessDesc ( gSelectedShaderIndex );
		for ( uint32_t bufferIndex = 0; bufferIndex < uniformAccessDesc.numActiveBindings; ++bufferIndex ) {
			ImGui::PushID ( bufferIndex + 100 );
			uint32_t gpuBindLocation = uniformAccessDesc.mGPUBindLocations[ bufferIndex ];
			uint32_t bufferAccessID = uniformAccessDesc.mBufferAccessIDs[ bufferIndex ];
			int32_t uniformLocation = uniformAccessDesc.mUniformLocations[ bufferIndex ];
			
			auto bufferAccessDesc = gShaderUniformManger.GetBufferAccessDesc ( gSelectedShaderIndex, bufferIndex );
			uint32_t glDataType = bufferAccessDesc.mDataType;
			//todo do we need name?

			int32_t len;
			glGetActiveUniformName ( shaderProgramHandle, gpuBindLocation, READ_BUFFER_SIZE, &len, gSharedReadBuffer );
			ImGui::Text ( "%s", gSharedReadBuffer );

			ImGui::Text ( "\t index: %d, gpuBind: %d, bufAccessID: %d, uniformLoc: %d", bufferIndex, gpuBindLocation, bufferAccessID, uniformLocation );
			auto dataTypeDesc = GetDataTypeDesc ( glDataType );
			ImGui::BulletText ( "Type: %s", dataTypeDesc.mName );

			DataTypeFlag dataTypeFlag = dataTypeDesc.mFlag;
			uint32_t dim = 1;
			bool isVec = dataTypeFlag & DataTypeFlags_VEC_TYPE;
			bool isMat = dataTypeFlag & DataTypeFlags_MAT_TYPE;
			if ( isVec )
			{
				for(uint32_t d = 2; d <= 4; ++d )
				{
					if ( dataTypeFlag & ( DataTypeFlags_VEC_TYPE << ( d - 1 ) ) )
					{
						dim = d;
					}
				}
			}
			if ( isMat )
			{
				for ( uint32_t d = 2; d <= 4; ++d ) {
					if ( dataTypeFlag & ( DataTypeFlags_MAT_TYPE << ( d - 1 ) ) ) {
						dim = d;
					}
				}
			}

			//assert()
			ImGuiDataType guiDataType = ImGuiDataType_Float;
			uint32_t iteration = isMat ? dim : 1;
			int32_t N = isMat || isVec ? dim : 1;
			char* pData = (char*)gShaderUniformManger.GetData(gSelectedShaderIndex, bufferIndex );
			assert ( pData );
			float fltRangeMin = 0.0f;
			float fltRangeMax = 1.0f;

			uint32_t dataSize = dataTypeDesc.mSize;
			for(uint32_t j = 0, offset = 0; j < iteration; ++j )
			{
				ImGui::PushID ( j );
				bool modified = ImGui::SliderScalarN ( "##Buffer", guiDataType, pData + offset, N, &fltRangeMin, &fltRangeMax );
				if(modified )
				{
					gShaderUniformManger.SetDirtyFlag(gSelectedShaderIndex, bufferIndex, true);
				}
				offset += ( dataSize / iteration );
				ImGui::PopID ();
			}

			ImGui::PopID ();
		}
	}
	ImGui::End ();


	example::NodeEditorShow ();

	glPushDebugGroup ( GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Draw UI" );
	// Render dear imgui into screen
	ImGui::Render ();
	ImGui_ImplOpenGL3_RenderDrawData ( ImGui::GetDrawData () );
	glPopDebugGroup ();

	glfwGetFramebufferSize ( pGLFWwindow,
							 &gAppScreenSize.x, &gAppScreenSize.y );
	glViewport ( 0, 0, gAppScreenSize.x, gAppScreenSize.y );

	glfwSwapBuffers ( pGLFWwindow );

	}
}

bool Application::Unload()
{
	return true;
}

void Application::Exit()
{
	glfwDestroyWindow(pGLFWwindow);
	glfwTerminate();
}
