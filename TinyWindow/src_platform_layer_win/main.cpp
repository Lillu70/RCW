
#include "../scr_user/Raycast_World.h"

#include "TW_App.h"

int WINAPI WinMain(HINSTANCE instance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	TW_App_Config config;
	config.title = "Back To The Past";
	TW_App app(instance, config);

	Raycast_World game(&app);

	while (app.get_is_running())
	{
		app.flush_events();

		game.update();
		
		app.update_surface();
	}

	return EXIT_SUCCESS;
}