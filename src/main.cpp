#include <iostream>

#include "Tools/Logger/Logger.h"
#include "Application/Application.h"
#include "Renderer/Renderer.h"
#include "Tools/Input/Input.h"

int main()
{
	gApplication.Init ();

	gRenderer.Init ();
	Input::Input::Input::Init ( gRenderer.GetGLFWwindow () );
	gRenderer.Load ();

	gApplication.Load ();

	//gInputManager.SetDisplaySize ( 1920, 1080 );

	while ( gApplication.ShouldUpdate () ) {
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
