#pragma once

#include "../TW_PLATFORM_INTERFACE.h"

#include "Pixel_Canvas.h"
#include "Components.h"
#include "Level.h"

class Raycast_World
{
public:
	Raycast_World(TW_Platform_Interface* platform);

	void update();
	void draw_top_down();
	void draw_first_person();
	
	//Takes in camera space coordinates, and returns true if the point lies inside the fov cone.
	static bool is_segment_on_screen(const v2f sp_p1, const v2f sp_p2, const f32 max_distance, const View& view);

private:
	View m_view;
	Level m_level;
	Pixel_Canvas m_canvas;
	static constexpr f32 MAX_DRAW_DISTANCE = 64;

	TW_Platform_Interface* m_platform = nullptr;
};


