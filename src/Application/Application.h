#pragma once
#include <string>
#include <vector>
#include <glm/glm.hpp>

class IScene;
class Scene;
struct Mesh;
//struct Geometry;

struct ApplicationInfo
{
	glm::ivec2 mScreenSize {};
	std::string mAppName {};
};

class Application
{
public:
	bool Init ();
	bool Load ();
	bool ShouldUpdate ();
	void Update ();
	void Draw ();
	void Cleanup ();
	void Unload ();


	std::vector<Mesh*> pMeshes;
	std::vector<unsigned> vaoID;

	ApplicationInfo GetAppInfo ();
private:


	IScene* pScene = NULL;
	ApplicationInfo mAppInfo;
};


extern Application gApplication;