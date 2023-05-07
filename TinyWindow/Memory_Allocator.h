#pragma once
#include "scr_user/RCW_ASSERT.h"
#include "Primitives.h"

struct Free_Block
{
	// Amount of bytes from the beging of the memory arena, to the start of this block.
	u32 offset = 0;

	// Amount of bytes available to this block.
	u32 size = 0;
};

struct Memory_Allocator
{
	void* memory = 0;
	u32 free_block_count = 0;

	// Byte count the allocator can hold.
	u32 capacity = 0;
};

static inline Free_Block* mem_alloc_get_free_list_being(Memory_Allocator* allocator)
{
	ASSERT(allocator);

	u32 offset_from_top = allocator->free_block_count * sizeof(Free_Block);
	Free_Block* block = (Free_Block*)((u8*)allocator->memory + (allocator->capacity - offset_from_top));

	ASSERT((void*)block >= allocator->memory);
	return block;
}

static inline void mem_alloc_init(Memory_Allocator* allocator, void* memory, u32 capacity, bool clear_to_zero = false)
{
	ASSERT(allocator);
	ASSERT(memory);
	ASSERT(capacity);

	allocator->capacity = capacity;
	allocator->memory = memory;
	allocator->free_block_count = 1;

	//Make sure the memory is cleared to zero.
	if (clear_to_zero)
		for (u8* i = (u8*)memory; i < (u8*)memory + capacity; ++i)
			*i = 0;

	Free_Block* free_block = mem_alloc_get_free_list_being(allocator);
	free_block->offset = 0;
	free_block->size = capacity - sizeof(Free_Block);
}

static inline void mem_alloc_shift_up_free_blocks(Free_Block* first_free, u32 start_idx)
{
	ASSERT(first_free);
	for (u32 i = start_idx; i > 0; --i)
		*(first_free + (i)) = *(first_free + (i - 1));

	// Zero is initialisation, where the first block used to be write in zeros.
	*(first_free) = { 0, 0 };
}

static inline void mem_alloc_merge_free_blocks(Memory_Allocator* allocator, Free_Block* first_free)
{
	ASSERT(allocator);
	ASSERT(first_free);

	for (u32 i = 0; i < allocator->free_block_count; ++i)
	{
		u8* bend = ((u8*)allocator->memory + first_free[i].offset + first_free[i].size);

		for (u32 y = 0; y < allocator->free_block_count; ++y)
		{
			if (i == y)
				continue;

			u8* bbeg = ((u8*)allocator->memory + first_free[y].offset);

			// Block i get's bigger.
			// Block y get's killed.
			// Begining of a block is the same as the next address after the end of anoter block, they can be merged. 
			if (bbeg == bend + 1)
			{
				allocator->free_block_count -= 1;
				first_free[i].size += first_free[y].size;

				mem_alloc_shift_up_free_blocks(first_free, y);
				i -= 1;
				first_free += 1;
				break;
			}
		}
	}
}

static inline void* mem_alloc_(Memory_Allocator* allocator, u32 size)
{
	ASSERT(allocator);
	ASSERT(size);

	// Size is of the block is stored at before the returened ptr, so a block size has to include the size of the size.
	size += sizeof(size);

	Free_Block* first_free = mem_alloc_get_free_list_being(allocator);
	for (u32 i = 0; i < allocator->free_block_count; ++i)
	{
		Free_Block* block = first_free + i;

		// Free block still has space, shrink it.
		if (size < block->size)
		{
			block->size -= size;
			block->offset += size;

			// Resort the array.
			// We don't need a full re-sort. We know that only this one, block has changed size and it has shrunk.

			Free_Block block_copy = *block;
			for (u32 y = i; y > 0; --y)
			{
				Free_Block* pb = (first_free + (y - 1));
				Free_Block* cb = (first_free + (y));
				if (pb->size > block_copy.size)
				{
					*cb = *pb;
					*pb = block_copy;
				}
				else
					break;
			}

			u32* size_insert_ptr = (u32*)((u8*)allocator->memory + block->offset);

			// True size of the block, including the size of block size.
			*size_insert_ptr = size;

			return (void*)(size_insert_ptr + 1);
		}

		// Free block is empty, kill it.
		else if (size == block->size)
		{
			allocator->free_block_count -= 1;
			Free_Block block_copy = *block;

			mem_alloc_shift_up_free_blocks(first_free, i);

			u32* size_insert_ptr = (u32*)((u8*)allocator->memory + block->offset);

			// True size of the block, including the size of block size.
			*size_insert_ptr = size;

			return (void*)(size_insert_ptr + 1);
		}
	}

	return nullptr;
}
# define push_struct(allocator, type) (type*)(mem_alloc_(allocator, sizeof(type)))