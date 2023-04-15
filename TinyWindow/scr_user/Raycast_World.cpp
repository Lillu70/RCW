#include "Raycast_World.h"
#include "RCW_ASSERT.h"

static constexpr i32 s_default_pixel_buffer_width	= 620;
static constexpr i32 s_default_pixel_buffer_height	= 480;


Raycast_World::Raycast_World(TW_Platform_Call_Table platform, void* game_state_memory, u32 game_state_memory_size) : 
	m_platform(platform), game_state((Game_State*)game_state_memory), m_game_state_memory_size(game_state_memory_size),
	m_canvas(platform.do_resize_pixel_buffer(s_default_pixel_buffer_width, s_default_pixel_buffer_height), { s_default_pixel_buffer_width, s_default_pixel_buffer_height })
{
	*game_state = Game_State();

	Wall_End* portal_4 = level.add_sector(v2f(10,10));
	level.add_wall(v2f(15, 10));
	Wall_End* portal_0 = level.add_wall(v2f(20, 10));
	level.add_wall(v2f(30, 10));
	level.add_wall(v2f(35, 30));
	level.add_wall(v2f(19, 20));
	
	Wall_End* portal_5 = level.add_sector(v2f(19, 20));
	level.add_wall(v2f(16, 30));
	level.add_wall(v2f(10, 10));

	level.add_sector(v2f(15, 10));
	Wall_End* portal_1 = level.add_wall(v2f(20, 10));
	Wall_End* portal_2 =level.add_wall(v2f(20, 0));
	level.add_wall(v2f(15, 0));

	level.add_sector(v2f(20, 10));
	Wall_End* portal_3 = level.add_wall(v2f(20, 0));
	level.add_wall(v2f(25, 5));

	level.make_wall_portal(portal_0, portal_1, true);
	level.make_wall_portal(portal_2, portal_3, true);
	level.make_wall_portal(portal_4, portal_5, true);

	level.m_sectors[2].floor = -100;
	level.m_sectors[2].ceiling = 700;
	level.m_sectors[3].floor = 100;
	level.m_sectors[3].ceiling = 300;

	game_state->player.sector = &level.m_sectors[0];
	View& player_view = game_state->player.view;
	player_view.position = { 24.34f, 21.65f };
	player_view.set_look_direction(1.95f);
	player_view.scale = 0.5;
	player_view.calculate_view_segments();
}

void Raycast_World::update()
{
	if (!m_platform.get_is_focused()) return;
	
	reset_is_drawn_flags();

	process_input();

	m_canvas.clear(put_color(0,0,0));
	
	draw_first_person();

	draw_top_down();
	

	m_canvas.draw_text("PLAYER: x: " + std::to_string(game_state->player.view.position.x) + " y: " + std::to_string(game_state->player.view.position.y), { 10,10 }, MAGENTA);
	m_canvas.draw_text("PLAYER: a: " + std::to_string(game_state->player.view.look_direction), { 10, 10 + m_canvas.s_build_in_font_char_height * 1 }, MAGENTA);
	
	m_canvas.draw_text("SECTOR: " + std::to_string((u64)(game_state->player.sector)), { 10, 10 + m_canvas.s_build_in_font_char_height * 2 }, MAGENTA);
	
}

void Raycast_World::process_input()
{
	if (m_platform.get_keyboard_state(Key_Code::F11).is_released())
		m_platform.set_fullscreen(!m_platform.get_is_fullscreen());

	Controller_State controller = m_platform.get_controller_state(0);
	f32 delta_time = m_platform.get_frame_time();

	if (m_platform.get_keyboard_state(Key_Code::ESC).is_released())
		m_platform.do_close();

	View& player_view = game_state->player.view;
	v2f player_pos = player_view.position;
	f32 turn_speed = game_state->player.turning_speed;

	f32 r_stick;
	if (controller.get_right_stick_x(r_stick))
	{
		player_view.set_look_direction(player_view.look_direction += delta_time * turn_speed * r_stick * -1);
	}
	else
	{
		if (m_platform.get_keyboard_button_down(Key_Code::LEFT))
			player_view.set_look_direction(player_view.look_direction += delta_time * turn_speed);

		if (m_platform.get_keyboard_button_down(Key_Code::RIGHT))
			player_view.set_look_direction(player_view.look_direction -= delta_time * turn_speed);
	}

	f32 movement_speed = game_state->player.movement_speed;

	f32 stick_val_x = 0; f32 stick_val_y = 0;
	bool stick_b_x = controller.get_left_stick_x(stick_val_x);
	bool stick_b_y = controller.get_left_stick_y(stick_val_y);

	if (stick_b_x || stick_b_y)
	{
		v2f movement_vector = { 0,0 };

		movement_vector.x += player_view.look_cos * stick_val_y * movement_speed * delta_time * -1;
		movement_vector.y += player_view.look_sin * stick_val_y * movement_speed * delta_time * -1;

		movement_vector.x -= cosf(player_view.look_direction + (f32)(PI / 2)) * stick_val_x * movement_speed * delta_time * -1;
		movement_vector.y -= sinf(player_view.look_direction + (f32)(PI / 2)) * stick_val_x * movement_speed * delta_time * -1;

		player_view.position.x += movement_vector.y;
		player_view.position.y += movement_vector.x;
	}
	else
	{
		v2f movement_vector = { 0,0 };

		if (controller.get_button_down(Button::DPAD_UP) || m_platform.get_keyboard_button_down(Key_Code::W))
		{
			movement_vector.x -= player_view.look_cos;
			movement_vector.y -= player_view.look_sin;
		}

		if (controller.get_button_down(Button::DPAD_DOWN) || m_platform.get_keyboard_button_down(Key_Code::S))
		{
			movement_vector.x += player_view.look_cos;
			movement_vector.y += player_view.look_sin;
		}

		if (controller.get_button_down(Button::DPAD_LEFT) || m_platform.get_keyboard_button_down(Key_Code::A))
		{
			movement_vector.x -= cosf(player_view.look_direction + (f32)(PI / 2));
			movement_vector.y -= sinf(player_view.look_direction + (f32)(PI / 2));
		}

		if (controller.get_button_down(Button::DPAD_RIGHT) || m_platform.get_keyboard_button_down(Key_Code::D))
		{
			movement_vector.x += cosf(player_view.look_direction + (f32)(PI / 2));
			movement_vector.y += sinf(player_view.look_direction + (f32)(PI / 2));
		}

		if (movement_vector != v2f(0, 0))
		{
			movement_vector = normalize(movement_vector);
			movement_vector = movement_vector * delta_time * movement_speed;
			player_view.position.x += movement_vector.y;
			player_view.position.y += movement_vector.x;
		}
	}

	if (player_pos != player_view.position && !game_state->player.sector->point_inside_sector(player_view.position))
	{
		bool sector_found = false;

		for(u32 i = 0; i < level.m_sector_count; i++)
			if (level.m_sectors[i].point_inside_sector(player_view.position))
			{
				game_state->player.sector = &level.m_sectors[i];
				sector_found = true;
				break;
			}

		if (!sector_found)
			player_view.position = player_pos;
	}

	/*
	Level& level = game_state->level;

	static f32 fheight = level.m_sectors[1].floor;
	static f32 cheight = level.m_sectors[1].ceiling;

	if (m_platform.get_keyboard_button_down(Key_Code::NUM_1))
		fheight -= delta_time * 100;
	if (m_platform.get_keyboard_button_down(Key_Code::NUM_2))
		fheight += delta_time * 100;

	if (m_platform.get_keyboard_button_down(Key_Code::NUM_3))
		cheight -= delta_time * 100;
	if (m_platform.get_keyboard_button_down(Key_Code::NUM_4))
		cheight += delta_time * 100;

	level.m_sectors[1].floor = fheight;
	level.m_sectors[1].ceiling = cheight;
	*/
}

void Raycast_World::draw_top_down()
{
	v2f half_screen = m_canvas.dimensions().As<f32>() / 2;

	View td_view = game_state->player.view;
	td_view.scale = 10.f;
	td_view.calculate_view_segments();

	m_canvas.draw_circle(half_screen.As<i32>(), 5, 3, 0xffff00ff);
	m_canvas.draw_line(half_screen.As<i32>(), v2f(half_screen.x, half_screen.y - 50).As<i32>(), 0xfff00fff);
	
	//Draw lines to vizualize the fov cone
	m_canvas.draw_line(half_screen.As<i32>(), v2f(half_screen.x + cosf(-(f32)PI / 2 - td_view.hfov) * td_view.max_draw_distance * td_view.scale,
		half_screen.y + sinf(-(f32)PI / 2 - td_view.hfov) * td_view.max_draw_distance * td_view.scale).As<i32>(), put_color(100, 100, 100));

	m_canvas.draw_line(half_screen.As<i32>(), v2f(half_screen.x + cosf(-(f32)PI / 2 + td_view.hfov) * td_view.max_draw_distance * td_view.scale,
		half_screen.y + sinf(-(f32)PI / 2 + td_view.hfov) * td_view.max_draw_distance * td_view.scale).As<i32>(), put_color(100, 100, 100));
	// ---

	for (u32 i = 0; i < level.m_sector_count; i++)
	{
		Sector& sector = level.m_sectors[i];
		if (sector.wall_count < 2)
			continue;

		v2f p1_world_pos = td_view.world_to_view((sector.first_wall + (sector.wall_count - 1))->world_pos);
		for (u32 w = 0; w < sector.wall_count; w++)
		{
			Wall_End& wall = *(sector.first_wall + w);
			
			//Convert wall points to top down view space.
			v2f p2 = td_view.world_to_view(wall.world_pos);
			
			u32 wcolor = put_color(100,100,100);

			if (wall.is_drawn)
				wcolor = put_color(255, 0, 255);

			m_canvas.draw_line((half_screen + (p1_world_pos)).As<i32>(), (half_screen + (p2)).As<i32>(), wcolor);
			p1_world_pos = p2;
		}
	}
}

void Raycast_World::draw_first_person()
{
	Sector*& player_sector = game_state->player.sector;
	
	ASSERT(player_sector);

	Render_Sector_Info info;
	info.left_y_bot = m_canvas.height() - 1;
	info.left_y_top = 0;
	info.right_y_bot = m_canvas.height() - 1;
	info.right_y_top = 0;
	info.x_max = m_canvas.width() - 1;
	info.x_min = 0;
	info.height_offset = 0;
	info.ignore_target = nullptr;
	info.source_floor = player_sector->floor;
	info.source_ceiling = player_sector->ceiling;
		
	draw_sector(player_sector, info);
}

void Raycast_World::draw_sector(Sector* sector, Render_Sector_Info info)
{
	View& view = game_state->player.view;

	if (sector->wall_count < 2)
		return;

	Wall_Info sector_walls[Level::s_sector_wall_max];

	i32 half_screen_height = (i32)m_canvas.height() / 2;

	sort_sector_walls(sector_walls, sector);

	for (u32 w = 0; w < sector->wall_count; w++)
	{
		Wall_Info wi = sector_walls[w];
		if (wi.wall == info.ignore_target)
			continue;

		v2f& p1 = wi.view_space_p1;
		v2f& p2 = wi.view_space_p2;
	
		//If both points are behind the view, continue.
		if (p1.y <= 0 && p2.y <= 0)
			continue;

		//p1 is the left most point.
		if (p2.x < p1.x)
			std::swap(p1, p2);

		//Initialize the angle to be outside of the fov.
		f32 r1 = view.fov;
		f32 r2 = view.fov;

		//If the point is in front of the camera, find the angle relative to the camera.
		if (p1.y > 0) r1 = abs(atanf(p1.x / p1.y));
		if (p2.y > 0) r2 = abs(atanf(p2.x / p2.y));

		//Both points lay outside of the fov cone.
		if (r1 > view.hfov && r2 > view.hfov)
		{
			if (p1.x > 0 || p2.x < 0 || !line_intersection({ 0,0 }, view.lv_segment, p1, p2, p1) || !line_intersection({ 0,0 }, view.rv_segment, p1, p2, p2))
				continue;
		}

		//left point is outside, but right point is inside.
		else if (r1 > view.hfov && r2 <= view.hfov)
		{
			if (!line_intersection({ 0,0 }, view.lv_segment, p1, p2, p1) && !line_intersection({ 0,0 }, view.rv_segment, p1, p2, p1))
				continue;
		}

		//right point is outside, but the left point is inside.
		else if (r1 <= view.hfov && r2 > view.hfov)
		{
			if (!line_intersection({ 0,0 }, view.lv_segment, p1, p2, p2) && !line_intersection({ 0,0 }, view.rv_segment, p1, p2, p2))
				continue;
		}

		//Recalculate angels to origin.
		r1 = atanf(p1.x / p1.y);
		r2 = atanf(p2.x / p2.y);

		//Calculate the start and end x values relative to screen size.
		i32 x1 = (i32)((0.5f + r1 / view.fov) * (m_canvas.width() - 1));
		i32 x2 = (i32)((0.5f + r2 / view.fov) * (m_canvas.width() - 1));

		if (x1 == x2)
			continue;

		const i32 sector_height = sector->height();

		const i32 h1 = (i32)(sector_height / p1.y) / 2;
		const i32 h2 = (i32)(sector_height / p2.y) / 2;

		const f32 eye_to_sector_ratio = (f32)(view.look_height + info.height_offset) / sector_height;

		v2i bot1{ x1, (i32)(half_screen_height + h1 * eye_to_sector_ratio) };
		v2i bot2{ x2, (i32)(half_screen_height + h2 * eye_to_sector_ratio) };
		v2i top1{ x1, bot1.y - h1 };
		v2i top2{ x2, bot2.y - h2 };

		//if x2 is on left side, swap points.
		if (x2 < x1)
		{
			std::swap(x1, x2);
			std::swap(top1, top2);
			std::swap(bot1, bot2);
		}

		//loop form left to right drawring linearly interpolated columns, between segment end points.
		const f32 m_top = slope(top1, top2);
		const f32 m_bot = slope(bot1, bot2);

		const f32 shading_factor = 1.f - std::min(0.2f, std::abs(m_top));
		const f32 floor_shading_factor = 1.f - ((f32)(info.source_floor - sector->floor) / 1000);
		const f32 ceiling_shading_factor = 1.f - ((f32)(info.source_ceiling - sector->ceiling) / 1000);

		u32 wall_color = multiply_accross_color_channels(wi.wall->color, shading_factor);
		
		u32 ceiling_color = multiply_accross_color_channels(put_color(0, 0, 178), ceiling_shading_factor);;
		u32 floor_color = multiply_accross_color_channels(put_color(0, 178, 0), floor_shading_factor);

		i32 x_start = std::max(x1, std::max(info.x_min, 0));
		i32 x_end	= std::min(x2, std::min(info.x_max, (i32)m_canvas.width() - 1));

		bool wall_not_drawn = true;

		if (wi.wall->portal)
		{
			i32 floor_dif = wi.wall->portal->floor - sector->floor;
			i32 ceiling_dif = sector->ceiling - wi.wall->portal->ceiling;
			f32 ceiling_ratio = (f32)ceiling_dif / sector_height;
			f32 floor_ratio = (f32)floor_dif / sector_height;
			
			//NOTE: Maybe checking that the floor is less than the ceiling shouled be an assert instead.
			if (floor_dif < sector_height && ceiling_dif < sector_height && wi.wall->portal->floor < wi.wall->portal->ceiling)
			{
				wall_not_drawn = false;

				Render_Sector_Info rsi;
				rsi.x_min = x1;
				rsi.x_max = x2;


				rsi.left_y_bot = bot1.y;  
				rsi.left_y_top = top1.y;   
				rsi.right_y_bot = bot2.y;  
				rsi.right_y_top = top2.y;  


				if (rsi.x_min < info.x_min)
				{
					rsi.x_min = info.x_min;
					rsi.left_y_top = linear_interpolation({ x1, top1.y }, { x2, top2.y }, info.x_min);
					rsi.left_y_bot = linear_interpolation({ x1, bot1.y }, { x2, bot2.y }, info.x_min);
					
				}	

				if (rsi.x_max > info.x_max)
				{
					rsi.x_max = info.x_max;
					rsi.right_y_top = linear_interpolation({ x1, top1.y }, { x2, top2.y }, info.x_max);
					rsi.right_y_bot = linear_interpolation({ x1, bot1.y }, { x2, bot2.y }, info.x_max);
				}
					
#
#if 1			//Funky hack!
				rsi.left_y_bot += 1;
				if(rsi.left_y_top > 0)
					rsi.left_y_top -= 1;
				rsi.right_y_bot += 1;
				if (rsi.right_y_top > 0)
					rsi.right_y_top -= 1;

#endif
				rsi.ignore_target = wi.wall->portal_wall;
				rsi.height_offset = info.height_offset + floor_dif * -1;
				rsi.source_floor = sector->floor;
				rsi.source_ceiling = sector->ceiling;

				draw_sector(wi.wall->portal, rsi);

				for (i32 x = x_start; x <= x_end; x++)
				{
					i32 y1 = (i32)(top1.y + m_top * (x - top1.x));
					i32 og_y1 = y1;
					i32 top_limit = round_to_int( linear_interpolation({ (f32)info.x_min, (f32)info.left_y_top }, { (f32)info.x_max, (f32)info.right_y_top }, (f32)x) );
					if (top_limit < 0)
						top_limit = 0;

					if (y1 < top_limit)
						y1 = top_limit;

					i32 y2 = (i32)(bot1.y + m_bot * (x - bot1.x));
					i32 og_y2 = y2;
					i32 bot_limit = round_to_int( linear_interpolation({ (f32)info.x_min, (f32)info.left_y_bot }, { (f32)info.x_max, (f32)info.right_y_bot }, (f32)x) );
					if (bot_limit > (i32)m_canvas.height() - 1)
						bot_limit = (i32)m_canvas.height() - 1;

					if (y2 > bot_limit)
						y2 = bot_limit;

					//draw ceiling
					if (y1 > top_limit)
						m_canvas.draw_vertical_column(x, top_limit, y1 - 1, ceiling_color);

					i32 true_column_height = og_y2 - og_y1;

					if (floor_dif > 0)
					{
						//Need to draw some of the wall if the other sector floor is higher or if the other sector ceiling is lower.
						i32 floor_wall_height = (i32)(true_column_height * floor_ratio);
						i32 floor_wall_y1 = og_y2 - floor_wall_height;
						if (floor_wall_y1 < y1)
							floor_wall_y1 = y1;

						i32 floor_wal_y2 = og_y2;
						if (floor_wal_y2 > bot_limit)
							floor_wal_y2 = bot_limit;
						if (floor_wal_y2 >= floor_wall_y1)
							m_canvas.draw_vertical_column(x, floor_wall_y1, floor_wal_y2, wall_color);
					}

					if (ceiling_dif > 0)
					{
						//Need to draw some of the wall if the other sector floor is higher or if the other sector ceiling is lower.
						i32 ceiling_wall_y1 = og_y1;
						if (ceiling_wall_y1 < y1)
							ceiling_wall_y1 = y1;

						i32 ceiling_wall_height = (i32)(true_column_height * ceiling_ratio);
						
						i32 ceiling_wal_y2 = og_y1 + ceiling_wall_height;
						if (ceiling_wal_y2 > bot_limit)
							ceiling_wal_y2 = bot_limit;

						if(ceiling_wal_y2 >= ceiling_wall_y1)
							m_canvas.draw_vertical_column(x, ceiling_wall_y1, ceiling_wal_y2, wall_color);
					}

					//draw floor
					if (y2 < bot_limit)
						m_canvas.draw_vertical_column(x, y2 + 1, bot_limit, floor_color);
				}
			}
		}
		
		if(wall_not_drawn)
		{
			for (i32 x = x_start; x <= x_end; x++)
			{
				i32 y1 = (i32)(top1.y + m_top * (x - top1.x));
				i32 top_limit = round_to_int( linear_interpolation({ (f32)info.x_min, (f32)info.left_y_top }, { (f32)info.x_max, (f32)info.right_y_top }, (f32)x) );
				if (top_limit < 0)
					top_limit = 0;

				if (y1 < top_limit)
					y1 = top_limit;

				i32 y2 = (i32)(bot1.y + m_bot * (x - bot1.x));
				i32 bot_limit = round_to_int( linear_interpolation({ (f32)info.x_min, (f32)info.left_y_bot }, { (f32)info.x_max, (f32)info.right_y_bot }, (f32)x) );
				if (bot_limit > (i32)m_canvas.height() - 1)
					bot_limit = (i32)m_canvas.height() - 1;

				if (y2 > bot_limit)
					y2 = bot_limit;

				if (y2 < top_limit)
					y2 = top_limit;
				

				//draw ceiling
				if (y1 > top_limit)
					m_canvas.draw_vertical_column(x, top_limit, y1 - 1, ceiling_color);

				//draw wall
				if (y1 < y2)
					m_canvas.draw_vertical_column(x, y1, y2, wall_color);

				//draw floor
				if (y2 < bot_limit)
					m_canvas.draw_vertical_column(x, y2 + 1, bot_limit, floor_color);

				y2 *= 10;
			}
		}

		wi.wall->is_drawn = true;
	}
}

void Raycast_World::sort_sector_walls(Wall_Info* output_buffer, Sector* sector)
{
	View& view = game_state->player.view;

	v2f p1_world_pos = flip_y(view.world_to_view((sector->first_wall + (sector->wall_count - 1))->world_pos));

	for (Wall_End* wp = sector->first_wall; wp < sector->first_wall + sector->wall_count; wp++)
	{
		Wall_Info wi;
		wi.wall = wp;
		
		wi.view_space_p1 = p1_world_pos;
		wi.view_space_p2 = flip_y(view.world_to_view(wp->world_pos));
		p1_world_pos = wi.view_space_p2;
		
		wi.depth = (wi.view_space_p1.y + wi.view_space_p2.y) / 2;


		//And idx is effectively the size of the sector_walls array as well.
		i32 idx = (u32)(wp - sector->first_wall);

		//Therefore by default we insert at the top.
		i32 insert_at = idx;

		for (i32 i = 0; i < idx; i++)
			if (output_buffer[i].depth < wi.depth)
			{
				insert_at = i;

				//Shift down all the elements with smaller depth value.
				for (i32 shift_idx = idx; shift_idx > i; shift_idx--)
					output_buffer[shift_idx] = output_buffer[shift_idx - 1];

				break;
			}

		output_buffer[insert_at] = wi;
	}
}

void Raycast_World::reset_is_drawn_flags()
{
	for (u32 i = 0; i < level.m_wall_count; ++i)
		level.m_walls[i].is_drawn = false;
	
}

bool Raycast_World::is_segment_on_screen(const v2f sp_p1, const v2f sp_p2, const f32 max_distance, const View& view)
{
	//Discard the segment, if both segment end points are behind the camera plane.
	if (sp_p1.y <= 0 && sp_p2.y <= 0) return false;

	//Initialize the angle to be outside of the fov.
	f32 r1 = view.fov;
	f32 r2 = view.fov;

	//If the point is in front of the camera, find the angle relative to the camera.
	if (sp_p1.y > 0) r1 = abs(atanf(sp_p1.x / sp_p1.y));
	if (sp_p2.y > 0) r2 = abs(atanf(sp_p2.x / sp_p2.y));

	//If one of the points lay inside of the camera field of view, then add the segment,
	//else if, both ends points lay outside of field of view.
	if (r1 > view.fov / 2 && r2 > view.fov / 2)
	{
		//Check if the segment crosses the field of view from outside of it.
		//Otherwise discard it.
		v2f hit;
		if (!line_intersection({ 0, 0 }, { 0, max_distance }, sp_p1, sp_p2, hit))
			return false;
	}

	return true;
}