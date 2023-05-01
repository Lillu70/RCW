#pragma once
#include "Types.h"
#include "RCW_ASSERT.h"
#include "Memory_Arena.h"


struct Sector
{
	Sector* next_sector = nullptr;
	i32 floor = 0;
	i32 ceiling = 480;
	u32 wall_count = 0;

	inline i32 height()
	{
		return ceiling - floor;
	}
};

struct Wall
{
	Wall() = default;
	Wall(v2f world_pos) : world_pos(world_pos) {}

	Sector* sector = nullptr;
	Wall* portal = nullptr;

	v2f world_pos;
	u32 color = WHITE;

	bool is_drawn = false;
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
	i32 source_floor;
	i32 source_ceiling;
	Wall* ignore_target;
};

struct Wall_Info
{
	f32 depth;
	v2f view_space_p1;
	v2f view_space_p2;
	Wall* wall;
};

struct Sector_Builder
{
	Sector_Builder(Memory_Arena* memory_arena) : mem_arena(memory_arena) {}

	inline Sector* add_sector()
	{
		if (last_sector)
		{
			ASSERT(last_sector->wall_count >= 3);
			
			last_sector->next_sector = push_struct_into_mem_arena(mem_arena, Sector);
			last_sector = last_sector->next_sector;
		}
		else
			last_sector = push_struct_into_mem_arena(mem_arena, Sector);
		
		next_address = mem_arena->next_free;
		
		return last_sector;
	};

	inline Wall* add_wall(Wall wall)
	{
		ASSERT(last_sector);
		ASSERT(next_address == mem_arena->next_free);

		++last_sector->wall_count;
		Wall* ptr = push_struct_into_mem_arena_with_placement(mem_arena, Wall, wall);
		ptr->sector = last_sector;
		next_address = mem_arena->next_free;

		return ptr;
	}

private:
	Memory_Arena* mem_arena = nullptr;
	Sector* last_sector = nullptr;
	void* next_address = nullptr;
};

static inline void make_wall_portal(Wall* source, Wall* target, bool make_bothways = false)
{
	source->portal = target;
	if (make_bothways)
		make_wall_portal(target, source);
}

static inline Wall* get_sector_first_wall(Sector* sector)
{
	return (Wall*)(++sector);
}

static inline bool point_inside_sector(v2f point, Sector* sector)
{
	ASSERT(sector);

	Wall* walls = get_sector_first_wall(sector);
	u32 wall_count = sector->wall_count;

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

