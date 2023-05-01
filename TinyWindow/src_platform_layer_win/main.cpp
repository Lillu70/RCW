
#include "../scr_user/Raycast_World.h"

#include "TW_App.cpp"


int WINAPI WinMain(HINSTANCE instance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	TW_App_Config config;
#if 0
	config.window_height = 480;
	config.window_width = 620;
#endif
	config.title = "Back To The Past";

	win32_init(instance, config);
	
	Raycast_World game(win32_get_call_table(), s_core.game_state_memory, GAME_STATE_MEMORY_SIZE);
	
	
	while (win32_get_is_running())
	{
		win32_flush_events();

		game.update();

		win32_update_surface();
	}
}