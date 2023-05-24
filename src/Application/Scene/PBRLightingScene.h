#pragma once
#include <vector>

#include "../Components/CameraController.h"
#include "IScene.h"

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
	void LoadGUI ();

	//For quad render
	unsigned emptyVAO{0};
	std::vector<Object*> pObjects;

	CameraController cameraController{};

	ubSceneLights sceneLights{};

	ShaderProgram* pGBufferFillShaderProgram = NULL;
	ShaderProgram* pBRDFLightingShaderProgram = NULL;

	unsigned cameraUBO {};
	unsigned objectUBO {};
	unsigned sceneLightsUBO {};

	unsigned gBufferFillFBO;
	unsigned gBuffer0;
	unsigned gBuffer1;
	unsigned gBuffer2;
	unsigned depthTextureHandle;


	unsigned shadowMapFBO;
	unsigned shadowMapTex;
	//Pipeline* pForwardPass;

	//Texture* pFinalTexture;
	
};


