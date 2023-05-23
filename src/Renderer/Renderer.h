#pragma once
#include <functional>
#include <vector>
struct Geometry;
struct GLFWwindow;
struct Mesh;

struct ShaderProgram
{
	int mHandle;
};

struct ShaderLoadDesc
{
	const char* vertexShaderName;
	const char* fragShaderName;
};

struct VAOLoadDesc
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
	} mAttributes[ 10 ];

	uint32_t numAttributes;
};

struct GUIControl
{
	bool showDemoWindow;
};

class Renderer
{
public:
	bool Init ();
	bool Load ();
	void Update ();
	void Draw ();
	void Unload ();

	unsigned LoadVAO ( VAOLoadDesc desc);
	unsigned MakeBuffer ();

	std::vector<ShaderProgram*> pShaderPrograms;
	GLFWwindow* const GetGLFWwindow () {
		return pGLFWwindow;
	}
private:
	bool LoadShader ( ShaderLoadDesc );
	bool GLFWInit ();

	GLFWwindow* pGLFWwindow = NULL;

	std::vector<Geometry*> pGeometries;

	GUIControl mGuiControl{};
};

extern Renderer gRenderer;

VAOLoadDesc MakeVAOLoadDesc ( Mesh* pMesh );

void RegisterGUIUpdateCallback ( std::function<void ()> callback );
