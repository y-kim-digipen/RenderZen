#include <iostream>

#include "Tools/Logger/Logger.h"
#include "Application/Application.h"
#include "Renderer/Renderer.h"
#include "Tools/Input/Input.h"

int main () {
	gApplication.Init ();

	gRenderer.Init ();
	Input::Input::Input::Init ( gRenderer.GetGLFWwindow () );
	gRenderer.Load ();

	gApplication.Load ();

	while ( gApplication.ShouldUpdate () ) {
		gApplication.Update ();
		gApplication.Draw ();
		gRenderer.Update ();
		gRenderer.Draw ();
	}

	gRenderer.Unload ();
	gApplication.Cleanup ();

	return 0;
}