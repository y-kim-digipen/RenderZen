#include "Application.h"

int main()
{
	Application app;
	bool initRes = app.Init();
	app.Load();
	app.Update();
	app.Unload();
	app.Exit();
	return 0;
}