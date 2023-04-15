#pragma once
#include "Types.h"

struct Sector;
struct Wall_End;

struct Render_Sector_Info
{
	i32 x_min;
	i32 x_max;
	i32 left_y_top;
	i32 left_y_bot;
	i32 right_y_top;
	i32 right_y_bot;
	i32 height_offset;
	i32 source_floor;
	i32 source_ceiling;
	Wall_End* ignore_target;
};


struct Wall_End
{
	Wall_End() = default;

	Wall_End(v2f p) : world_pos(p) {}
	Wall_End(v2f p, u32 c) : world_pos(p), color(c) {}

	v2f world_pos;
	u32 color = WHITE;
	Sector* portal = nullptr;
	Wall_End* portal_wall = nullptr;
	bool is_drawn = false;
};

struct Wall_Info
{
	f32 depth;
	v2f view_space_p1;
	v2f view_space_p2;
	Wall_End* wall;
};

struct Sector
{
	Wall_End* first_wall;
	u32 wall_count;

	i32 floor	= 0;
	i32 ceiling = 480;

	inline i32 height()
	{
		return ceiling - floor;
	}

	//https://www.codeproject.com/Tips/84226/Is-a-Point-inside-a-Polygon
	inline bool point_inside_sector(v2f point)
	{
		Wall_End*& walls = first_wall;

		i32 i, j;
		bool c = false;
		for (i = 0, j = (i32)wall_count - 1; i < (i32)wall_count; j = i++)
		{
			if (((walls[i].world_pos.y > point.y) != (walls[j].world_pos.y > point.y)) &&
				(point.x < (walls[j].world_pos.x - walls[i].world_pos.x) * (point.y - walls[i].world_pos.y) / (walls[j].world_pos.y - walls[i].world_pos.y) + walls[i].world_pos.x))
				c = !c;
		}
		return c;
	}
};

struct Level
{
	static constexpr u32 s_wall_max = 64;
	static constexpr u32 s_sector_wall_max = 16;
	static constexpr u32 s_sector_max = 32;
	
	u32 m_sector_count = 0;
	u32 m_wall_count = 0;

	Wall_End m_walls[s_wall_max];
	Sector m_sectors[s_sector_max];

	Sector* get_wall_sector(Wall_End* wall);
	Wall_End* add_wall(Wall_End wall);
	Wall_End* add_sector(Wall_End first_wall);
	void make_wall_portal(Wall_End* source, Wall_End* target, bool make_bothways); 
};