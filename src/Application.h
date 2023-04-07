#pragma once


struct GLFWwindow;

class Application
{
public:
	bool Load();
	bool Init();
	void Update();
	bool Unload();
	void Exit();
private:
};