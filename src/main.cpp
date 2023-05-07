#include "App/Application.h"
#include "App/Renderer.h"
#include "App/Input.h"

//gainput::InputManager gInputManager;

int main()
{
	gApplication.Init ();

	gRenderer.Init ();
	Input::Input::Init ( gRenderer.GetGLFWwindow () );
	gRenderer.Load ();

	gApplication.Load ();

	//gInputManager.SetDisplaySize ( 1920, 1080 );

	while(gApplication.ShouldUpdate() )
	{
		//gInputManager.Update ();
		gApplication.Update ();
		gApplication.Draw ();
		gRenderer.Update ();
		gRenderer.Draw ();
	}

	//gApplication.Unload ();
	gRenderer.Unload ();
	gApplication.Cleanup ();

	return 0;
}