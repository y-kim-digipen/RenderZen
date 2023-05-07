#pragma once
#include <vector>

#include "CameraController.h"
#include "IScene.h"
#include "../../visual_studio_2019/Engine/Light.h"

struct ShaderProgram;
struct Object;
struct Camera;

class DeferredPassTestScene final : public IScene
{
public:
	virtual bool Init () override;
	virtual bool Load () override;
	virtual void Update ( float dt ) override;
	virtual void Draw () override;
	virtual void Unload () override;
	virtual void Cleanup () override;

private:
	void LoadGBuffers ();
	bool LoadShadowMap ();
	void LoadGUI ();

	//For quad render
	unsigned emptyVAO{0};
	std::vector<Object*> pObjects;

	CameraController cameraController{};

	ShaderProgram* pDeferredPassShaderProgram = NULL;
	ShaderProgram* pCelShadingShaderProgram = NULL;

	unsigned cameraUBOID {};
	unsigned directionalLightUBOID {};

	unsigned FBO;
	unsigned positionTextureHandle;
	unsigned normalTextureHandle;
	unsigned colorTextureHandle;
	unsigned depthTextureHandle;




	unsigned shadowMapFBO;
	unsigned shadowMapTex;
	//Pipeline* pForwardPass;

	//Texture* pFinalTexture;
	DirectionalLight directionalLight{};
	
};


