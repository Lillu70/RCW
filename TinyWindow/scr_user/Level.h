#pragma once
#include "Types.h"
#include "Components.h"

#include <array>

struct Sector;



struct Wall
{
	Wall() = default;
	Wall(v2f p1, v2f p2) : p1(p1), p2(p2) {}
	Wall(v2f p1, v2f p2, u32 c) : p1(p1), p2(p2), color(c) {}
	Wall(v2f p1, v2f p2, Sector* portal, Wall* portal_wall) : portal(portal) , portal_wall(portal_wall) {}

	v2f p1, p2;
	u32 color = WHITE;

	Sector* portal = nullptr;
	Wall* portal_wall = nullptr;
};

struct Wall_Info
{
	f32 depth;
	v2f view_space_p1;
	v2f view_space_p2;
	Wall* wall;
};

struct Render_Sector_Info
{
	i32 x_min;
	i32 x_max;
	i32 left_y_top;
	i32 left_y_bot;
	i32 right_y_top;
	i32 right_y_bot;
	i32 height_offset;
	Wall* ignore_target;
};

struct Sector
{
	Wall* first_wall;
	u32 wall_count;

	i32 floor	= 000;
	i32 ceiling = 480;

	inline i32 height()
	{
		return ceiling - floor;
	}
};

class Level
{
public:
	static constexpr u32 s_wall_max = 64;
	static constexpr u32 s_sector_wall_max = 16;
	static constexpr u32 SECTOR_MAX = 32;
	
	u32 m_sector_count = 0;
	u32 m_wall_count = 0;

	std::array<Wall, s_wall_max> m_walls;
	std::array<Sector, SECTOR_MAX> m_sectors;
};

