#pragma once
#include "Maths.h"
#include "Level.h"
#include "Memory_Arena.h"

struct View
{
	View()
	{
		calculate_view_segments();
	}

	v2f position = {0,0};
	f32 look_cos = 0;
	f32 look_sin = 0;
	f32 fov = (f32)(PI / 2);
	f32 hfov = fov / 2;
	f32 look_direction = 0;
	f32 scale = 1;
	i32 look_height = 480 / 3;
	i32 max_draw_distance = 256;
	v2f lv_segment;
	v2f rv_segment;

	void set_fov(f32 new_fov)
	{
		fov = new_fov;
		hfov = fov / 2;
		calculate_view_segments();
	}

	void set_look_direction(f32 d)
	{
		//while (d < 0) d += TAU32;
		//while (d > TAU) d -= TAU32;
		
		look_direction = d;
		look_cos = cosf( d );
		look_sin = sinf( d );
	}

	inline v2f world_to_view(const v2f wp)
	{
		//return flip_y( rotate_point(wp * scale - position * scale, look_cos, look_sin) );
		return rotate_point(wp * scale - position * scale, look_cos, look_sin );
	}

	void calculate_view_segments()
	{
		lv_segment = v2f(cosf((-hfov - ((f32)PI / 2))) * max_draw_distance * scale, abs(sinf(-hfov - ((f32)PI / 2))) * max_draw_distance * scale);
		rv_segment = v2f(cosf((+hfov - ((f32)PI / 2))) * max_draw_distance * scale, abs(sinf(+hfov - ((f32)PI / 2))) * max_draw_distance * scale);
	}
};

struct Player
{
	Sector* sector;
	View view;

	f32 movement_speed = 10.f;
	f32 turning_speed = 3.f;
};

