#include "Raycast_World.h"
#include <iostream>

static constexpr i32 s_default_pixel_buffer_width = 620;
static constexpr i32 s_default_pixel_buffer_height = 480;

Raycast_World::Raycast_World(TW_Platform_Interface* platform) : m_platform(platform), m_canvas(platform->do_resize_pixel_buffer(s_default_pixel_buffer_width, s_default_pixel_buffer_height),
	{ s_default_pixel_buffer_width, s_default_pixel_buffer_height })
{
	m_level.m_walls[m_level.m_wall_count++] = Wall({ 10,10 }, { 15,10 }, WHITE);
	m_level.m_walls[m_level.m_wall_count++] = Wall({ 15,10 }, { 20,10 }, put_color(128,0,128));
	Wall* portal_wall = &m_level.m_walls[m_level.m_wall_count - 1];

	m_level.m_walls[m_level.m_wall_count++] = Wall({ 20,10 }, { 30,10 }, WHITE);
	m_level.m_walls[m_level.m_wall_count++] = Wall({ 30,10 }, { 30,30 }, WHITE);
	m_level.m_walls[m_level.m_wall_count++] = Wall({ 30,30 }, { 17,20 }, WHITE);
	m_level.m_walls[m_level.m_wall_count++] = Wall({ 17,20 }, { 12,22 }, WHITE);
	m_level.m_walls[m_level.m_wall_count++] = Wall({ 12,22 }, { 10,10 }, WHITE);
	m_level.m_sectors[m_level.m_sector_count++] = { m_level.m_walls.data(), m_level.m_wall_count };

	Wall* sector_start = &m_level.m_walls[m_level.m_wall_count];
	m_level.m_walls[m_level.m_wall_count++] = Wall({ 15,10 }, { 20,10 }, put_color(128, 0, 128));
	m_level.m_walls[m_level.m_wall_count++] = Wall({ 20,10 }, { 20, 0 }, WHITE);
	m_level.m_walls[m_level.m_wall_count++] = Wall({ 20,0 },  { 12, 0 }, WHITE);	
	m_level.m_walls[m_level.m_wall_count++] = Wall({ 12,0 },  { 15, 10 }, WHITE);

	m_level.m_sectors[m_level.m_sector_count++] = 
		{ sector_start, m_level.m_wall_count - (m_level.m_sectors[0].wall_count - 1) };
	
	portal_wall->portal = &m_level.m_sectors[1];
	portal_wall->portal_wall = sector_start;

	m_player_sector = &m_level.m_sectors[0];

	m_view.position = { 15,15 };
	m_view.set_look_direction((f32)m_view.look_direction);
	m_view.scale = 0.5;
	m_view.calculate_view_segments();
}

void Raycast_World::update()
{
	if (!m_platform->get_is_focused()) return;
	
	Controller_State controller = m_platform->get_controller_state();
	u8 input = 0;
	f32 delta_time = m_platform->get_frame_time();

	if (m_platform->get_keyboard_state(Key_Code::ESC).is_released())
		m_platform->do_close();

	f32 turn_speed = 3.f;

	f32 r_stick;
	if (controller.get_right_stick_x(r_stick))
	{
		m_view.set_look_direction(m_view.look_direction += delta_time * turn_speed * r_stick * -1);
	}
	else
	{
		if (m_platform->get_keyboard_button_down(Key_Code::LEFT))
			m_view.set_look_direction(m_view.look_direction += delta_time * turn_speed);

		if (m_platform->get_keyboard_button_down(Key_Code::RIGHT))
			m_view.set_look_direction(m_view.look_direction -= delta_time * turn_speed);
	}
	
	f32 movement_speed = 5.f;

	f32 stick_val_x = 0; f32 stick_val_y = 0;
	bool stick_b_x = controller.get_left_stick_x(stick_val_x);
	bool stick_b_y = controller.get_left_stick_y(stick_val_y);
	
	if (stick_b_x || stick_b_y)
	{
		v2f movement_vector = { 0,0 };

		movement_vector.x += m_view.look_cos * stick_val_y * movement_speed * delta_time * -1;
		movement_vector.y += m_view.look_sin * stick_val_y * movement_speed * delta_time * -1;
	
		movement_vector.x -= cosf(m_view.look_direction + (f32)(PI / 2)) * stick_val_x * movement_speed * delta_time * -1;
		movement_vector.y -= sinf(m_view.look_direction + (f32)(PI / 2)) * stick_val_x * movement_speed * delta_time * -1;
		
		m_view.position.x += movement_vector.y;
		m_view.position.y += movement_vector.x;
	}
	else
	{
		v2f movement_vector = { 0,0 };

		if (controller.get_button_down(Button::DPAD_UP) || m_platform->get_keyboard_button_down(Key_Code::W))
		{
			movement_vector.x -= m_view.look_cos;
			movement_vector.y -= m_view.look_sin;
		}

		if (controller.get_button_down(Button::DPAD_DOWN) || m_platform->get_keyboard_button_down(Key_Code::S))
		{
			movement_vector.x += m_view.look_cos;
			movement_vector.y += m_view.look_sin;
		}

		if (controller.get_button_down(Button::DPAD_LEFT) || m_platform->get_keyboard_button_down(Key_Code::A))
		{
			movement_vector.x -= cosf(m_view.look_direction + (f32)(PI / 2));
			movement_vector.y -= sinf(m_view.look_direction + (f32)(PI / 2));
		}

		if (controller.get_button_down(Button::DPAD_RIGHT) || m_platform->get_keyboard_button_down(Key_Code::D))
		{
			movement_vector.x += cosf(m_view.look_direction + (f32)(PI / 2));
			movement_vector.y += sinf(m_view.look_direction + (f32)(PI / 2));
		}

		if (movement_vector != v2f(0, 0))
		{
			movement_vector = normalize(movement_vector);
			movement_vector = movement_vector * delta_time * movement_speed;
			m_view.position.x += movement_vector.y;
			m_view.position.y += movement_vector.x;
		}
	}


	m_canvas.clear(0);

	draw_first_person();
	draw_top_down();
}

void Raycast_World::draw_top_down()
{
	v2f half_screen = m_canvas.dimensions().As<f32>() / 2;

	View td_view = m_view;
	td_view.scale = 10;
	td_view.calculate_view_segments();

	m_canvas.draw_circle(half_screen.As<i32>(), 5, 3, 0xffff00ff);
	m_canvas.draw_line(half_screen.As<i32>(), v2f(half_screen.x, half_screen.y - 50).As<i32>(), 0xfff00fff);
	
	//Draw lines to vizualize the fov cone
	m_canvas.draw_line(half_screen.As<i32>(), v2f(half_screen.x + cosf(-(f32)PI / 2 - m_view.hfov) * MAX_DRAW_DISTANCE * td_view.scale,
		half_screen.y + sinf(-(f32)PI / 2 - m_view.hfov) * MAX_DRAW_DISTANCE * td_view.scale).As<i32>(), put_color(100, 100, 100));

	m_canvas.draw_line(half_screen.As<i32>(), v2f(half_screen.x + cosf(-(f32)PI / 2 + m_view.hfov) * MAX_DRAW_DISTANCE * td_view.scale,
		half_screen.y + sinf(-(f32)PI / 2 + m_view.hfov) * MAX_DRAW_DISTANCE * td_view.scale).As<i32>(), put_color(100, 100, 100));
	// ---


	for (u32 i = 0; i < m_level.m_sector_count; i++)
	{
		Sector& sector = m_level.m_sectors[i];
		for (u32 w = 0; w < sector.wall_count; w++)
		{
			Wall& wall = *(sector.first_wall + w);
			
			//Convert wall points to top down view space.
			v2f p1 = td_view.world_to_view(wall.p1);
			v2f p2 = td_view.world_to_view(wall.p2);
			u32 wcolor = put_color(100,100,100);

			if (is_segment_on_screen(p1, p2, MAX_DRAW_DISTANCE * td_view.scale, td_view))
				wcolor = put_color(255, 0, 255);

			m_canvas.draw_line((half_screen + flip_y(p1)).As<i32>(), (half_screen + flip_y(p2)).As<i32>(), wcolor);
		}
	}
}

void Raycast_World::draw_first_person()
{
	Render_Sector_Info info;
	info.left_y_bot = m_canvas.height() - 1;
	info.left_y_top = 0;
	info.right_y_bot = m_canvas.height() - 1;
	info.right_y_top = 0;
	info.x_max = m_canvas.width() - 1;
	info.x_min = 0;
	info.height_offset = 0;
	info.ignore_target = nullptr;

	draw_sector(*m_player_sector, info);
}

void Raycast_World::draw_sector(Sector& sector, Render_Sector_Info info)
{
	std::array<Wall_Info, Level::s_sector_wall_max> sector_walls;

	i32 half_screen_height = (i32)m_canvas.height() / 2;

	sort_sector_walls(sector_walls, sector);

	for (u32 w = 0; w < sector.wall_count; w++)
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
		f32 r1 = m_view.fov;
		f32 r2 = m_view.fov;

		//If the point is in front of the camera, find the angle relative to the camera.
		if (p1.y > 0) r1 = abs(atanf(p1.x / p1.y));
		if (p2.y > 0) r2 = abs(atanf(p2.x / p2.y));

		//Both points lay outside of the fov cone.
		if (r1 > m_view.hfov && r2 > m_view.hfov)
		{
			if (p1.x > 0 || p2.x < 0 || !line_intersection({ 0,0 }, m_view.lv_segment, p1, p2, p1) || !line_intersection({ 0,0 }, m_view.rv_segment, p1, p2, p2))
				continue;
		}

		//left point is outside, but right point is inside.
		else if (r1 > m_view.hfov && r2 <= m_view.hfov)
		{
			if (!line_intersection({ 0,0 }, m_view.lv_segment, p1, p2, p1) && !line_intersection({ 0,0 }, m_view.rv_segment, p1, p2, p1))
				continue;
		}

		//right point is outside, but the left point is inside.
		else if (r1 <= m_view.hfov && r2 > m_view.hfov)
		{
			if (!line_intersection({ 0,0 }, m_view.lv_segment, p1, p2, p2) && !line_intersection({ 0,0 }, m_view.rv_segment, p1, p2, p2))
				continue;
		}

		//Recalculate angels to origin.
		r1 = atanf(p1.x / p1.y);
		r2 = atanf(p2.x / p2.y);

		//Calculate the start and end x values relative to screen size.
		i32 x1 = (i32)((0.5f + r1 / m_view.fov) * m_canvas.width());
		i32 x2 = (i32)((0.5f + r2 / m_view.fov) * m_canvas.width());

		if (x1 == x2)
			continue;

		const i32 h1 = (i32)(sector.height() / p1.y) / 2;
		const i32 h2 = (i32)(sector.height() / p2.y) / 2;

		const f32 eye_to_sector_ratio = (f32)m_view.look_height / sector.height();

		v2i bot1{ x1, (i32)(half_screen_height + h1 * eye_to_sector_ratio+ info.height_offset) };
		v2i bot2{ x2, (i32)(half_screen_height + h2 * eye_to_sector_ratio+ info.height_offset) };
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

		for (i32 x = std::max(x1, info.x_min); x < std::min(x2, info.x_max); x++)
		{
			i32 y1 = (i32)(top1.y + m_top * (x - top1.x));
			i32 lit = linear_interpolation({ info.x_min, info.left_y_top }, { info.x_max, info.right_y_top }, x);
	
			
			i32 y2 = (i32)(bot1.y + m_bot * (x - bot1.x));
			i32 lib = linear_interpolation({ info.x_min, info.left_y_bot }, { info.x_max, info.right_y_bot }, x);
	
			u32 wall_color = multiply_accross_color_channels(wi.wall->color, shading_factor);
			u32 ceiling_color = put_color(0, 0, 255);
			u32 floor_color = put_color(0, 255, 0);
			
			m_canvas.draw_vertical_column(x, lit, lib, y1, y2, ceiling_color, wall_color, floor_color);
		}

		if (wi.wall->portal)
		{
			Render_Sector_Info rsi;
			rsi.x_min = x1;
			rsi.x_max = x2;
			rsi.left_y_bot = bot1.y;
			rsi.left_y_top = top1.y;
			rsi.right_y_bot = bot2.y;
			rsi.right_y_top = top2.y;
			rsi.height_offset = 0;
			rsi.ignore_target = wi.wall->portal_wall;

			draw_sector(*wi.wall->portal, rsi);
		}
	}
}

void Raycast_World::sort_sector_walls(std::array<Wall_Info, Level::s_sector_wall_max>& output, Sector& sector)
{
	for (Wall* wp = sector.first_wall; wp < sector.first_wall + sector.wall_count; wp++)
	{
		Wall_Info wi;
		wi.wall = wp;
		wi.view_space_p1 = m_view.world_to_view(wp->p1);
		wi.view_space_p2 = m_view.world_to_view(wp->p2);
		wi.depth = (wi.view_space_p1.y + wi.view_space_p2.y) / 2;

		//And idx is effectively the size of the sector_walls array as well.
		i32 idx = (u32)(wp - sector.first_wall);

		//Therefore by default we insert at the top.
		i32 insert_at = idx;

		for (i32 i = 0; i < idx; i++)
			if (output[i].depth < wi.depth)
			{
				insert_at = i;

				//Shift down all the elements with smaller depth value.
				for (u32 shift_idx = idx; shift_idx > i; shift_idx--)
					output[shift_idx] = output[shift_idx - 1];

				break;
			}

		output[insert_at] = wi;
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