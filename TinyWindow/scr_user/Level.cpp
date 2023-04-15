#include "Level.h"
#include "RCW_ASSERT.h"

Sector* Level::get_wall_sector(Wall_End* wall)
{
	for (u32 sector_idx = 0; sector_idx < m_sector_count; ++sector_idx)
		if (wall >= m_sectors[sector_idx].first_wall && wall < m_sectors[sector_idx].first_wall + m_sectors[sector_idx].wall_count)
			return &m_sectors[sector_idx];

	abort();
}

Wall_End* Level::add_wall(Wall_End wall)
{
	ASSERT(m_wall_count + 1 <= s_wall_max)

	m_walls[m_wall_count] = wall;
	m_sectors[m_sector_count - 1].wall_count += 1;
	return &m_walls[m_wall_count++];
}

Wall_End* Level::add_sector(Wall_End first_wall)
{
	ASSERT(m_sector_count + 1 <= s_sector_max)

	m_walls[m_wall_count] = first_wall;
	m_sectors[m_sector_count].first_wall = &m_walls[m_wall_count++];
	m_sectors[m_sector_count].wall_count += 1;

	return m_sectors[m_sector_count++].first_wall;
}

void Level::make_wall_portal(Wall_End* source, Wall_End* target, bool make_bothways = true)
{
	source->portal = get_wall_sector(target);
	source->portal_wall = target;

	if (make_bothways)
		make_wall_portal(target, source, false);
}

