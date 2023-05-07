#pragma once
#include <vector>
#include "IScene.h"
#include "CameraController.h"
//Interface class of scene, every scene using stencil
//buffer cube rendering pipeline should inherit this class
//Scene
//
//Camera
//Object
//
//Pipeline1...
//Pipeline2...
//
//FinalTexture

struct ShaderProgram;
struct Object;
struct Camera;

class M1Scene final : public IScene
{
public:
	virtual bool Init () override;
	virtual bool Load () override;
	virtual void Update (float dt) override;
	virtual void Draw () override;
	virtual void Unload () override;
	virtual void Cleanup () override;

private:
	void LoadGUI ();

	std::vector<Object*> pObjects;
	//Camera* pCurrentCamera = NULL;
	cbCamera camera{};

	ShaderProgram* pForwardPassShaderProgram = NULL;

	unsigned cbCameraUBOID{};
	//Pipeline* pForwardPass;

	//Texture* pFinalTexture;
};