#pragma once

#include "../TW_PLATFORM_INTERFACE.h"

#include "Pixel_Canvas.h"
#include "Components.h"

class TW_Platform_Interface;

class Raycast_World
{
public:
	Raycast_World(TW_Platform_Call_Table platform, void* game_state_memory, u32 game_state_memory_size);

	void update();
	void process_input();
	void draw_top_down();
	void draw_first_person();
	
	void draw_sector(Sector* sector, Render_Sector_Info info);
	void sort_sector_walls(Wall_Info* output_buffer, Sector* sector);

	void reset_is_drawn_flags();
	
	//Takes in camera space coordinates, and returns true if the point lies inside the fov cone.
	static bool is_segment_on_screen(const v2f sp_p1, const v2f sp_p2, const f32 max_distance, const View& view);

private:
	Pixel_Canvas m_canvas;
	Game_State* game_state = nullptr;
	Level& level = game_state->level;
	u32 m_game_state_memory_size = 0;

	TW_Platform_Call_Table m_platform;
};
