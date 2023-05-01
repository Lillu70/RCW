#pragma once

#include "../Primitives.h"
#include "RCW_ASSERT.h"

struct Memory_Arena
{
	void* start = 0;
	void* next_free = 0;
	u32 capasity = 0;
};

static void* push_struct_into_mem_arena_(Memory_Arena* arena, u32 struct_size)
{
	ASSERT(arena);
	ASSERT((u8*)arena->next_free - (u8*)arena->start + struct_size < arena->capasity);
	void* placement_location = arena->next_free;
	arena->next_free = (u8*)arena->next_free + struct_size;

	return placement_location;
}
#define push_struct_into_mem_arena(arena, type) &(*(type*)push_struct_into_mem_arena_(arena, sizeof(type))=type())
#define push_struct_into_mem_arena_with_placement(arena, type, placement) &(*(type*)push_struct_into_mem_arena_(arena, sizeof(type))=placement)

static inline void init_memory_arena(Memory_Arena* arena, void* memory_start, u32 memory_capasity)
{
	ASSERT(memory_capasity > 0);
	ASSERT(memory_start);
	ASSERT(arena);

	arena->start = memory_start;
	arena->next_free = memory_start;
	arena->capasity = memory_capasity;
}

static inline u32 memory_arena_available_space(Memory_Arena* arena)
{
	if (arena->next_free == arena->start)
		return arena->capasity;

	return (u32)(arena->capasity - (((u8*)arena->next_free - 1) - (u8*)arena->start));
}

static inline void memory_arena_clear(Memory_Arena* arena)
{
	ASSERT(arena);
	for (u8* ptr = (u8*)arena->start; ptr < (u8*)arena->next_free; ++ptr)
		*ptr = 0;

	arena->next_free = arena->start;
}