#pragma once
#include "Maths.h"



struct View
{
	v2f position = {0,0};
	f32 look_cos = 0;
	f32 look_sin = 0;
	f32 fov = (f32)(PI / 2);
	f32 look_direction = 0;
	f32 scale = 1;
	i32 look_height = 480 / 3;

	void set_look_direction(f32 d)
	{
		while (d < 0) d += (f32)TAU;
		while (d > TAU) d -= (f32)TAU;
		
		look_direction = d;
		look_cos = cosf( d );
		look_sin = sinf( d );
	}

	inline v2f world_to_view(const v2f wp)
	{
		return flip_y( rotate_point(wp * scale - position * scale, look_cos, look_sin) );
	}
};

