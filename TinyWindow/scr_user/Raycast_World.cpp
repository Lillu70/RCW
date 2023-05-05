#include "Raycast_World.h"
#include "RCW_ASSERT.h"

static constexpr i32 s_default_pixel_buffer_width	= 620;
static constexpr i32 s_default_pixel_buffer_height	= 480;

static constexpr i32 s_screen_ref_width = s_default_pixel_buffer_width;
static constexpr i32 s_screen_ref_height = s_default_pixel_buffer_height;

Raycast_World::Raycast_World(TW_Platform_Call_Table platform, void* game_state_memory, u32 game_state_memory_size) : m_platform(platform),
	m_canvas(platform.do_resize_pixel_buffer(s_default_pixel_buffer_width, s_default_pixel_buffer_height), { s_default_pixel_buffer_width, s_default_pixel_buffer_height })
{
	init_memory_arena(&m_game_memory, game_state_memory, game_state_memory_size);
	
	m_player = push_struct_into_mem_arena(&m_game_memory, Player);

#if 0
	u32 window_width = m_platform.get_window_width();
	u32 window_height = m_platform.get_window_height();

	m_platform.output_debug_string(("window width: " + std::to_string(window_width) + "\n").c_str());
	m_platform.output_debug_string(("window height: " + std::to_string(window_height) + "\n").c_str());

	m_canvas = Pixel_Canvas(m_platform.do_resize_pixel_buffer(window_width, window_height), {window_width, window_height});
#endif


	Sector_Builder builder(&m_game_memory);
	m_first_sector = builder.add_sector();

#if 1
	Wall* portal_0 = builder.add_wall(v2f{ 10, 10 });
	builder.add_wall(v2f{ 15, 10 });
	Wall* portal_1 = builder.add_wall(v2f{ 20, 10 });
	builder.add_wall(v2f{ 30, 10 });
	builder.add_wall(v2f{ 35, 30 });
	builder.add_wall(v2f{ 19, 20 });
	
	builder.add_sector();
	Wall* portal_2 = builder.add_wall(v2f{ 19, 20 });
	builder.add_wall(v2f{ 16, 30 });
	builder.add_wall(v2f{ 10, 10 });
	
	{
		Sector* sector = builder.add_sector();
		sector->floor = -100;
		sector->ceiling = 700;
	}
	builder.add_wall(v2f{ 15, 10 });
	Wall* portal_3 = builder.add_wall(v2f{ 20, 10 });
	Wall* portal_4 = builder.add_wall(v2f{ 20, 0 });
	builder.add_wall(v2f{ 15, 0 });
	
	{
		Sector* sector = builder.add_sector();
		sector->floor = 100;
		sector->ceiling = 300;
	}
	builder.add_wall(v2f{ 20, 10 });
	Wall* portal_5 = builder.add_wall(v2f{ 20, 0 });
	builder.add_wall(v2f{ 25, 5 });

	make_wall_portal(portal_0, portal_2, true);
	make_wall_portal(portal_1, portal_3, true);
	make_wall_portal(portal_4, portal_5, true);

#else
	builder.add_wall(v2f{ 10, 10 });
	builder.add_wall(v2f{ 30, 10 });
#endif

	m_player->sector = m_first_sector;
	m_player->position = { 15, 15 };
	{
		View& player_view = m_player->view;
		player_view.set_look_direction(deg_to_rad(0));
		player_view.scale = 0.5;
		player_view.set_fov(PI32 / 3);
		player_view.calculate_view_segments();
	}
}

void Raycast_World::update()
{
	if (!m_platform.get_is_focused()) return;
	
	Controller_State controller = m_platform.get_controller_state(0);
	f32 delta_time = m_platform.get_frame_time();

	process_input(&controller, delta_time);

	handle_player_movement(&controller, delta_time);

	m_canvas.clear(MAGENTA);

	draw_first_person();

	static bool draw_debug = true;
	if (m_platform.get_keyboard_state(Key_Code::F1).is_released())
		draw_debug = !draw_debug;
	if (draw_debug)
	{
		draw_top_down();
		draw_debug_info();
	}
}

void Raycast_World::handle_player_movement(Controller_State* controller, f32 delta_time)
{
	f32 t = delta_time;

	{ //Player rotation
		f32 la = 0;
		
		f32 r_stick;
		if (controller->get_right_stick_x(r_stick))
			la = -r_stick;
		
		else
		{
			if (m_platform.get_keyboard_button_down(Key_Code::LEFT))
				la = 1.f;

			if (m_platform.get_keyboard_button_down(Key_Code::RIGHT))
				la = -1.f;
		}

		if (la != 0)
		{
			la *= 15.f;
			la += m_player->turning_velocity * -5.f;
			f32 l1 = m_player->view.look_direction;
			f32 lv = la * t + m_player->turning_velocity;
			m_player->turning_velocity = lv;

			f32 l = la * 0.5f * square(t) + lv * t + l1;
			m_player->view.set_look_direction(l);
		}
		else
			m_player->turning_velocity = 0.f;
		
	}

	//P1 (position) (1/2)a*(t*t)+v*t+p<(old position)
	//P2 (velocity) a*t+v<(old velocity)
	//P3 (acceleration) input
	v2f a = get_acceleration_vector(controller) * m_player->acceleration_speed;
	a += -m_player->velocity * 5.f;

	v2f p1 = m_player->position;
	v2f v = a * t + m_player->velocity;
	m_player->velocity = v;

	v2f p = a * 0.5f * square(t) + v * t + p1;

	Sector* sector = m_player->sector;
	Wall* first_wall = get_sector_first_wall(sector);
	
	// super simple collision and sector detection
	//TODO: Better collision system than this shite.
	if (!point_inside_sector(p, sector))
	{
		bool player_in_valid_sector = false;
		for (Wall* w = first_wall; w < first_wall + sector->wall_count; ++w)
		{
			if (w->portal && point_inside_sector(p, w->portal->sector))
			{
				player_in_valid_sector = true;
				m_player->sector = w->portal->sector;
			}
		}

		if (!player_in_valid_sector)
			p = m_player->position;
		
	}
	m_player->position = p;
	

	m_player->view.position = m_player->position;
}


v2f Raycast_World::get_acceleration_vector(Controller_State* controller)
{
	v2f acceleration = 0;

	f32 stick_val_x = 0;
	f32 stick_val_y = 0;
	bool stick_b_x = controller->get_left_stick_x(stick_val_x);
	bool stick_b_y = controller->get_left_stick_y(stick_val_y);
	if (stick_b_x || stick_b_y)
	{
		acceleration.y += m_player->view.look_cos * -stick_val_y;
		acceleration.x += m_player->view.look_sin * -stick_val_y;
		
		acceleration.y -= cosf(m_player->view.look_direction + (f32)(PI / 2)) * -stick_val_x;
		acceleration.x -= sinf(m_player->view.look_direction + (f32)(PI / 2)) * -stick_val_x;
	}
	else
	{
		if (controller->get_button_down(Button::DPAD_UP) || m_platform.get_keyboard_button_down(Key_Code::W))
		{
			acceleration.y -= m_player->view.look_cos;
			acceleration.x -= m_player->view.look_sin;
		}

		if (controller->get_button_down(Button::DPAD_DOWN) || m_platform.get_keyboard_button_down(Key_Code::S))
		{
			acceleration.y += m_player->view.look_cos;
			acceleration.x += m_player->view.look_sin;
		}

		if (controller->get_button_down(Button::DPAD_LEFT) || m_platform.get_keyboard_button_down(Key_Code::A))
		{
			acceleration.y -= cosf(m_player->view.look_direction + (f32)(PI / 2));
			acceleration.x -= sinf(m_player->view.look_direction + (f32)(PI / 2));
		}

		if (controller->get_button_down(Button::DPAD_RIGHT) || m_platform.get_keyboard_button_down(Key_Code::D))
		{
			acceleration.y += cosf(m_player->view.look_direction + (f32)(PI / 2));
			acceleration.x += sinf(m_player->view.look_direction + (f32)(PI / 2));
		}
	}

	return acceleration;
}

void Raycast_World::process_input(Controller_State* controller, f32 delta_time)
{
	if (m_platform.get_keyboard_state(Key_Code::F11).is_released())
		m_platform.set_fullscreen(!m_platform.get_is_fullscreen());

	if (m_platform.get_keyboard_state(Key_Code::ESC).is_released())
		m_platform.do_close();

	View& player_view = m_player->view;

	if (controller->get_button_down(Button::L_SHLD) || m_platform.get_keyboard_button_down(Key_Code::F5))
	{
		f32 new_fov = player_view.fov - delta_time;
		new_fov = max(PI32/10, new_fov);
		player_view.set_fov(new_fov);
	}

	if (controller->get_button_down(Button::R_SHLD) || m_platform.get_keyboard_button_down(Key_Code::F6))
	{
		f32 new_fov = player_view.fov + delta_time;
		new_fov = min(PI32 - PI32 / 10, new_fov);
		player_view.set_fov(new_fov);
	}
}

void Raycast_World::draw_top_down()
{
	v2f half_screen = m_canvas.dimensions().As<f32>() / 2;

	View td_view = m_player->view;
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

	Sector* sector = m_first_sector;
	while (sector)
	{
		if (sector->wall_count < 2)
		{
			sector = sector->next_sector;
			continue;
		}

		v2f p1_world_pos = td_view.world_to_view((get_sector_first_wall(sector) + (sector->wall_count - 1))->world_pos);
		for (u32 w = 0; w < sector->wall_count; w++)
		{
			Wall& wall = *(get_sector_first_wall(sector) + w);

			//Convert wall points to top down view space.
			v2f p2 = td_view.world_to_view(wall.world_pos);

			u32 wcolor = put_color(100, 100, 100);

			if (wall.is_drawn)
			{
				if (wall.portal)
					wcolor = put_color(0, 255, 0);
				else
					wcolor = put_color(255, 0, 255);
			}
			wall.is_drawn = false;

			m_canvas.draw_line((half_screen + (p1_world_pos)).As<i32>(), (half_screen + (p2)).As<i32>(), wcolor);
			p1_world_pos = p2;
		}

		sector = sector->next_sector;
	}
}

void Raycast_World::draw_first_person()
{
	Memory_Arena draw_mem;
	init_memory_arena(&draw_mem, m_game_memory.next_free, memory_arena_available_space(&m_game_memory));

	Sector*& player_sector = m_player->sector;
	
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
		
	draw_sector(player_sector, info, &draw_mem);
	
	memory_arena_clear(&draw_mem);
}

void Raycast_World::draw_debug_info()
{
	v2i pos = { 10,10 };
	u32 col = MAGENTA;

	m_canvas.draw_text("PLAYER: x: " + std::to_string(m_player->position.x) + " y: " + std::to_string(m_player->position.y), pos, col);
	pos.y += m_canvas.s_build_in_font_char_height;
	

	static bool look_direction_in_radians = true;
	if (m_platform.get_keyboard_state(Key_Code::G).is_pressed())
		look_direction_in_radians = !look_direction_in_radians;
	if (look_direction_in_radians)
	{
		m_canvas.draw_text("PLAYER: a (RAD): " + std::to_string(m_player->view.look_direction), pos, col);
		pos.y += m_canvas.s_build_in_font_char_height;
		m_canvas.draw_text("PLAYER: FOV (RAD): " + std::to_string(m_player->view.fov), pos, col);
	}
	else
	{
		m_canvas.draw_text("PLAYER: a (DEG): " + std::to_string(rad_to_deg(m_player->view.look_direction)), pos, col);
		pos.y += m_canvas.s_build_in_font_char_height;
		m_canvas.draw_text("PLAYER: FOV (DEG): " + std::to_string(rad_to_deg(m_player->view.fov)), pos, col);
	}
	pos.y += m_canvas.s_build_in_font_char_height;

	m_canvas.draw_text("VEL: x: " + std::to_string(m_player->velocity.x) + " y: " + std::to_string(m_player->velocity.x), pos, col);
	pos.y += m_canvas.s_build_in_font_char_height;
}

void Raycast_World::draw_sector(Sector* sector, Render_Sector_Info info, Memory_Arena* mem_arena)
{
	View& view = m_player->view;

	if (sector->wall_count < 2)
		return;

	Wall_Info* sector_walls = (Wall_Info*)push_struct_into_mem_arena_(mem_arena, sizeof(Wall_Info)*sector->wall_count);

	i32 screen_height = (i32)m_canvas.height();
	i32 half_screen_height = screen_height / 2;
	i32 screen_width = (i32)m_canvas.width();

	sort_sector_walls(sector_walls, sector);

	for (u32 w = 0; w < sector->wall_count; w++)
	{
		Wall_Info wi = sector_walls[w];	

		//These points come in; in view space, that is different from screen space.
		//View space is just relative to the camera position/orientation, but dimensions are still in world space,
		//and effectively top down; meaning that y is treated like z in 3D.
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
		//Covert x values from view space into pixel space.
		i32 x1 = round_to_int((r1 / view.fov + 0.5f) * (screen_width - 1));
		i32 x2 = round_to_int((r2 / view.fov + 0.5f) * (screen_width - 1));
		ASSERT(x1 < screen_width && x2 < screen_width);


		if (x1 == x2)
			continue;

		if (wi.wall == info.ignore_target)
		{
			wi.wall->is_drawn = true;
			continue;
		}

		i32 sector_height = sector->height();

		f32 h1 = (f32)sector_height / p1.y;
		f32 h2 = (f32)sector_height / p2.y;

		f32 eye_to_sector_ratio = (f32)(view.look_height + info.height_offset) / sector_height;

		v2i bot1{ x1, (i32)(half_screen_height + round_to_int(h1 * eye_to_sector_ratio + 0.5f)) }; 
		v2i bot2{ x2, (i32)(half_screen_height + round_to_int(h2 * eye_to_sector_ratio + 0.5f)) };  
		v2i top1{ x1, round_to_int(bot1.y - h1 - 0.5f) };										     
		v2i top2{ x2, round_to_int(bot2.y - h2 - 0.5f) };										     
	
		
		//if x2 is on left side, swap points.
		if (x2 < x1)
		{
			std::swap(x1, x2);
			std::swap(top1, top2);
			std::swap(bot1, bot2);
		}

		//loop form left to right drawring linearly interpolated columns, between segment end points.
		f32 m_top = slope(top1, top2);
		f32 m_bot = slope(bot1, bot2);

		f32 shading_factor = 1.f - min(0.2f, std::abs(m_top));
		f32 floor_shading_factor = 1.f - ((f32)(m_player->view.look_height - sector->floor) / 1000);
		f32 ceiling_shading_factor = 1.f - ((f32)(m_player->view.look_height - sector->ceiling) / 1000);

		u32 wall_color = multiply_accross_color_channels(wi.wall->color, shading_factor);
		
		u32 ceiling_color = multiply_accross_color_channels(put_color(0, 0, 178), ceiling_shading_factor);;
		u32 floor_color = multiply_accross_color_channels(put_color(0, 178, 0), floor_shading_factor);

		i32 x_start = max(x1, max(info.x_min, 0));
		i32 x_end	= min(x2, min(info.x_max, screen_width - 1));

		bool wall_not_drawn = true;

		if (wi.wall->portal)
		{
			i32 portal_floor = wi.wall->portal->sector->floor;
			i32 portal_ceiling = wi.wall->portal->sector->ceiling;

			i32 floor_dif = portal_floor - sector->floor;
			i32 ceiling_dif = sector->ceiling - portal_ceiling;
			f32 ceiling_ratio = (f32)ceiling_dif / sector_height;
			f32 floor_ratio = (f32)floor_dif / sector_height;
			
			//NOTE: Maybe checking that the floor is less than the ceiling shouled be an assert instead.
			if (floor_dif < sector_height && ceiling_dif < sector_height && portal_floor < portal_ceiling)
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
				rsi.left_y_bot += 2;
				//if(rsi.left_y_top > 0)
				rsi.left_y_top -= 2;
				rsi.right_y_bot += 2;
				//if (rsi.right_y_top > 0)
				rsi.right_y_top -= 2;

#endif
				rsi.ignore_target = wi.wall->portal;
				rsi.height_offset = info.height_offset + floor_dif * -1;
				rsi.source_floor = sector->floor;
				rsi.source_ceiling = sector->ceiling;

				draw_sector(wi.wall->portal->sector, rsi, mem_arena);

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
					if (bot_limit > screen_height - 1)
						bot_limit = screen_height - 1;

					if (y2 > bot_limit)
						y2 = bot_limit;

					//draw ceiling
					if (y1 > top_limit && y1 < bot_limit)
						m_canvas.draw_vertical_column(x, top_limit, y1 - 1, ceiling_color);

					i32 true_column_height = og_y2 - og_y1;

					if (floor_dif > 0)
					{
						//Need to draw some of the wall if the other sector floor is higher or if the other sector ceiling is lower.
						i32 floor_wall_height = round_to_int((f32)true_column_height * floor_ratio);
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

						i32 ceiling_wall_height = round_to_int((f32)true_column_height * ceiling_ratio);
						
						i32 ceiling_wal_y2 = og_y1 + ceiling_wall_height;
						if (ceiling_wal_y2 > bot_limit)
							ceiling_wal_y2 = bot_limit;

						if(ceiling_wal_y2 >= ceiling_wall_y1)
							m_canvas.draw_vertical_column(x, ceiling_wall_y1, ceiling_wal_y2, wall_color);
					}

					//draw floor
					if (y2 < bot_limit && y2 >= -1)
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

				i32 y2 = (i32)(bot1.y + m_bot * (x - bot1.x));
				i32 bot_limit = round_to_int( linear_interpolation({ (f32)info.x_min, (f32)info.left_y_bot }, { (f32)info.x_max, (f32)info.right_y_bot }, (f32)x) );
				if (bot_limit > screen_height - 1)
					bot_limit = screen_height - 1;

				if (y1 < top_limit)
					y1 = top_limit;

				if (y2 > bot_limit)
					y2 = bot_limit;

				if (y2 < top_limit)
					y2 = top_limit;
				
				if (y1 < y2)
				{
					//draw ceiling
					if (y1 > top_limit)
						m_canvas.draw_vertical_column(x, top_limit, y1 - 1, ceiling_color);

					//draw wall
					if (y1 < y2)
						m_canvas.draw_vertical_column(x, y1, y2, wall_color);

					//draw floor
					if (y2 < bot_limit)
						m_canvas.draw_vertical_column(x, y2 + 1, bot_limit, floor_color);
				}
			}
		}

		wi.wall->is_drawn = true;
	}
}

void Raycast_World::sort_sector_walls(Wall_Info* output_buffer, Sector* sector)
{
	View& view = m_player->view;

	v2f p1_world_pos = flip_y(view.world_to_view((get_sector_first_wall(sector) + (sector->wall_count - 1))->world_pos));

	for (Wall* wp = get_sector_first_wall(sector); wp < get_sector_first_wall(sector) + sector->wall_count; wp++)
	{
		Wall_Info wi;
		wi.wall = wp;
		
		wi.view_space_p1 = p1_world_pos;
		wi.view_space_p2 = flip_y(view.world_to_view(wp->world_pos));
		p1_world_pos = wi.view_space_p2;
		
		wi.depth = (wi.view_space_p1.y + wi.view_space_p2.y) / 2;


		//And idx is effectively the size of the sector_walls array as well.
		i32 idx = (u32)(wp - get_sector_first_wall(sector));

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