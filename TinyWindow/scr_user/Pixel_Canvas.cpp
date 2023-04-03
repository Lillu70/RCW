#include "Pixel_Canvas.h"



Pixel_Canvas::Pixel_Canvas(u32* pixels, v2u dimensions) : m_pixels(pixels), m_dimensions(dimensions)
{

}

Pixel_Canvas::~Pixel_Canvas()
{

}


void Pixel_Canvas::clear_random()
{
	for (u32 i = 0; i < m_dimensions.x * m_dimensions.y; i++)
		m_pixels[i] = put_color((u8)rand() % 255, (u8)rand() % 255, (u8)rand() % 255);
}

void Pixel_Canvas::clear(u32 color)
{
	for (u32 i = 0; i < m_dimensions.x * m_dimensions.y; i++)
		m_pixels[i] = color;
}

void Pixel_Canvas::set_pixel(v2i coord, u32 color)
{
	if (is_on_canvas(coord))
		m_pixels[coord_to_idx(v2u((u32)coord.x, (u32)coord.y))] = color;
}

void Pixel_Canvas::draw_filled_circle(v2i center, u32 radius, u32 color)
{
	v2u start;
	start.x = center.x - radius;
	start.y = center.y - radius;
	if (start.x > m_dimensions.x) start.x = 0;
	if (start.y > m_dimensions.y) start.y = 0;

	v2u end;
	end.x = center.x + radius;
	end.y = center.y + radius;
	if (end.x >= m_dimensions.x) end.x = m_dimensions.x - 1;
	if (end.y >= m_dimensions.y) end.y = m_dimensions.y - 1;

	for (u32 y = start.y; y <= end.y; y++)
		for (u32 x = start.x; x <= end.x; x++)
			if (distance(center, { (i32)x,(i32)y }) <= radius)
				m_pixels[coord_to_idx({ x,y })] = color;
}

void Pixel_Canvas::draw_circle(v2i center, u32 radius, u32 thickness, u32 color)
{
	v2u start;
	start.x = center.x - radius;
	start.y = center.y - radius;
	if (start.x > m_dimensions.x) start.x = 0;
	if (start.y > m_dimensions.y) start.y = 0;

	v2u end;
	end.x = center.x + radius;
	end.y = center.y + radius;
	if (end.x >= m_dimensions.x) end.x = m_dimensions.x - 1;
	if (end.y >= m_dimensions.y) end.y = m_dimensions.y - 1;

	u32 half_thickness = thickness / 2;
	for (u32 y = start.y; y <= end.y; y++)
		for (u32 x = start.x; x <= end.x; x++)
		{
			f32 d = distance(center, { (i32)x,(i32)y });
			if (d >= (f32)(radius - half_thickness) && d <= (f32)(radius + half_thickness))
				m_pixels[coord_to_idx({ x,y })] = color;
		}
			
}

void Pixel_Canvas::draw_line(v2i p1, v2i p2, u32 color)
{
	f64 angle = std::atan2((f64)p2.y - p1.y, (f64)p2.x - p1.x);
	if(angle < (PI / -4.0) || angle >= PI - (PI/4.0))
		std::swap(p1, p2);

	i32 xlen = std::abs((p2.x - p1.x));
	i32 ylen = std::abs((p2.y - p1.y));
	f32 m = slope(p1.As<f32>(), p2.As<f32>());

	if (xlen >= ylen)
		for (i32 x = 0; x <= xlen; x++) 
			set_pixel(v2i( p1.x + x, p1.y + (i32)(m * x) ), color);
	else
		for (i32 y = 0; y <= ylen; y++)
			set_pixel(v2i( p1.x + (i32)(y / m), p1.y + y ), color);
}

void Pixel_Canvas::draw_filled_rect(v2i p1, v2i p2, u32 color)
{
	v2i sp1 = { std::min(p1.x, p2.x), std::min(p1.y, p2.y) };
	v2i sp2 = { std::max(p1.x, p2.x), std::max(p1.y, p2.y) };

	for (i32 y = sp1.y; y < sp2.y; y++)
		for (i32 x = sp1.x; x < sp2.x; x++)
			set_pixel({ x,y }, color);
}

void Pixel_Canvas::draw_rect(v2i p1, v2i p2, i32 thickeness, u32 color)
{
	//left to right,
	draw_filled_rect(p1, { p2.x, p1.y + thickeness }, color);
	
	//right to down,
	draw_filled_rect({ p2.x - thickeness, p1.y + thickeness }, p2, color);

	//down right to left,
	draw_filled_rect({ p1.x, p2.y - thickeness }, {p2.x - thickeness, p2.y}, color);

	//down right to left,
	draw_filled_rect({ p1.x, p1.y + thickeness }, { p1.x + thickeness, p2.y - thickeness }, color);
}

void Pixel_Canvas::draw_vertical_column(i32 x, i32 region_top, i32 region_bot, i32 column_top, i32 column_bot, u32 top_color, u32 column_color, u32 bot_color)
{
	if (x < 0 || x >= (i32)m_dimensions.x) return;
	
	//Bigger of; SCREEN_HEIHT, collumn_bot!

	i32 og_y1 = column_top;
	i32 og_y2 = column_bot;

	region_top = std::max(0, region_top);
	region_bot = std::min((i32)m_dimensions.y -1, region_bot);
	

	column_bot = std::min(region_bot, column_bot);
	column_top = std::max(region_top, column_top);

	for (i32 y = region_top; y < column_top; ++y)
		m_pixels[coord_to_idx({ (u32)x, (u32)y })] = top_color;
	
	for (i32 y = column_top; y <= column_bot; ++y)
		m_pixels[coord_to_idx({ (u32)x, (u32)y })] = column_color;
	
	for (i32 y = column_bot; y < region_bot; ++y)
		m_pixels[coord_to_idx({ (u32)x, (u32)y })] = bot_color;

	//for (i32 y = std::max(y_top, 0); y < std::min(y_bot, (i32)(m_dimensions.y - 1)); y++)
	//	m_pixels[coord_to_idx({ (u32)x, (u32)y })] = color;
}

bool Pixel_Canvas::is_on_canvas(v2i coord)
{
	return (coord.x < 0 || coord.y < 0 || coord.x >= (i32)m_dimensions.x || coord.y >= (i32)m_dimensions.y)? false : true;
}

inline bool Pixel_Canvas::is_on_canvas(u32 pixel_idx)
{
	return pixel_idx < m_dimensions.x * m_dimensions.y;
}

inline u32 Pixel_Canvas::coord_to_idx(v2u coord)
{
	return coord.y * m_dimensions.x + coord.x;
}



static constexpr unsigned char s_build_in_font[] =
{
	  0,   0,  48,  48,  48,  48,  48,  48,  48,   0,  48,  48,   0,   0,   0, 204, 204, 204,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	108, 108, 108, 254, 108, 108, 254, 108, 108, 108,   0,   0,   0,  16,  16, 124, 214,  22,  22, 124, 208, 208, 214, 124,  16,  16,   0,   0, 204, 214,
	108,  96,  48,  48,  24, 216, 172, 204,   0,   0,   0,   0,  56, 108, 108,  56, 220, 118, 102, 102, 119, 220,   0,   0,   0,  48,  48,  48,   0,   0,
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  96,  48,  24,  24,  24,  24,  24,  24,  48,  96,   0,   0,   0,   0,  24,  48,  96,  96,  96,  96,
	 96,  96,  48,  24,   0,   0,   0,   0,   0,   0,   0, 108,  56, 254,  56, 108,   0,   0,   0,   0,   0,   0,   0,   0,   0,  48,  48, 252,  48,  48,
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  48,  48,  24,   0,   0,   0,   0,   0,   0,   0,   0, 254,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  48,  48,   0,   0,   0,   0, 192, 192,  96,  96,  48,  48,  24,  24,  12,  12,   0,   0,
	  0,   0, 124, 198, 198, 230, 246, 222, 206, 198, 198, 124,   0,   0,   0,   0,  48,  56,  60,  48,  48,  48,  48,  48,  48, 252,   0,   0,   0,   0,
	124, 198, 198, 192,  96,  48,  24,  12,   6, 254,   0,   0,   0,   0, 124, 198, 198, 192, 120, 192, 192, 198, 198, 124,   0,   0,   0,   0, 192, 224,
	240, 216, 204, 198, 254, 192, 192, 192,   0,   0,   0,   0, 254,   6,   6,   6, 126, 192, 192, 192, 198, 124,   0,   0,   0,   0, 120,  12,   6,   6,
	126, 198, 198, 198, 198, 124,   0,   0,   0,   0, 254, 192, 192,  96,  96,  48,  48,  24,  24,  24,   0,   0,   0,   0, 124, 198, 198, 198, 124, 198,
	198, 198, 198, 124,   0,   0,   0,   0, 124, 198, 198, 198, 198, 252, 192, 192,  96,  60,   0,   0,   0,   0,   0,   0,   0,  48,  48,   0,   0,   0,
	 48,  48,   0,   0,   0,   0,   0,   0,   0,  48,  48,   0,   0,   0,  48,  48,  24,   0,   0,   0,   0, 192,  96,  48,  24,  12,  24,  48,  96, 192,
	  0,   0,   0,   0,   0,   0,   0, 254,   0,   0, 254,   0,   0,   0,   0,   0,   0,   0,   0,  12,  24,  48,  96, 192,  96,  48,  24,  12,   0,   0,
	  0,   0, 124, 198, 198, 198,  96,  48,  48,   0,  48,  48,   0,   0,   0,   0, 124, 198, 230, 214, 214, 214, 214, 230,   6, 252,   0,   0,   0,   0,
	124, 198, 198, 198, 198, 254, 198, 198, 198, 198,   0,   0,   0,   0, 126, 198, 198, 198, 126, 198, 198, 198, 198, 126,   0,   0,   0,   0, 124, 198,
	198,   6,   6,   6,   6, 198, 198, 124,   0,   0,   0,   0,  62, 102, 198, 198, 198, 198, 198, 198, 102,  62,   0,   0,   0,   0, 254,   6,   6,   6,
	 62,   6,   6,   6,   6, 254,   0,   0,   0,   0, 254,   6,   6,   6,  62,   6,   6,   6,   6,   6,   0,   0,   0,   0, 124, 198, 198,   6,   6, 246,
	198, 198, 198, 124,   0,   0,   0,   0, 198, 198, 198, 198, 254, 198, 198, 198, 198, 198,   0,   0,   0,   0, 120,  48,  48,  48,  48,  48,  48,  48,
	 48, 120,   0,   0,   0,   0, 240,  96,  96,  96,  96,  96,  96, 102, 102,  60,   0,   0,   0,   0, 198, 198, 102,  54,  30,  30,  54, 102, 198, 198,
	  0,   0,   0,   0,   6,   6,   6,   6,   6,   6,   6,   6,   6, 254,   0,   0,   0,   0, 130, 198, 238, 254, 214, 198, 198, 198, 198, 198,   0,   0,
	  0,   0, 198, 198, 198, 206, 222, 246, 230, 198, 198, 198,   0,   0,   0,   0, 124, 198, 198, 198, 198, 198, 198, 198, 198, 124,   0,   0,   0,   0,
	126, 198, 198, 198, 198, 126,   6,   6,   6,   6,   0,   0,   0,   0, 124, 198, 198, 198, 198, 198, 198, 198, 246, 124, 192,   0,   0,   0, 126, 198,
	198, 198, 198, 126,  30,  54, 102, 198,   0,   0,   0,   0, 124, 198,   6,   6, 124, 192, 192, 198, 198, 124,   0,   0,   0,   0, 254,  48,  48,  48,
	 48,  48,  48,  48,  48,  48,   0,   0,   0,   0, 199, 198, 198, 198, 198, 198, 198, 198, 198, 124,   0,   0,   0,   0, 198, 198, 198, 198, 198, 108,
	108, 108,  56,  56,   0,   0,   0,   0, 198, 198, 198, 198, 198, 214, 254, 238, 198, 130,   0,   0,   0,   0, 198, 198, 108, 108,  56,  56, 108, 108,
	198, 198,   0,   0,   0,   0, 134, 134, 204, 204, 120,  48,  48,  48,  48,  48,   0,   0,   0,   0, 255, 193, 192,  96,  48,  24,  12,   6,   6, 254,
	  0,   0,   0,   0, 120,  24,  24,  24,  24,  24,  24,  24,  24, 120,   0,   0,   0,   0,  12,  12,  24,  24,  48,  48,  96,  96, 192, 192,   0,   0,
	  0,   0, 120,  96,  96,  96,  96,  96,  96,  96,  96, 120,   0,   0,   0,  48, 120, 204,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 254,  24,  48,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	  0, 124, 192, 252, 198, 198, 198, 252,   0,   0,   0,   0,   6,   6,   6, 126, 198, 198, 198, 198, 198, 126,   0,   0,   0,   0,   0,   0,   0, 124,
	198,   6,   6,   6, 198, 124,   0,   0,   0,   0, 192, 192, 192, 252, 198, 198, 198, 198, 198, 252,   0,   0,   0,   0,   0,   0,   0, 124, 198, 198,
	254,   6,   6, 124,   0,   0,   0,   0, 240,  24,  24, 126,  24,  24,  24,  24,  24,  24,   0,   0,   0,   0,   0,   0,   0, 252, 198, 198, 198, 198,
	198, 252, 192, 192,   0,   0,   6,   6,   6, 126, 198, 198, 198, 198, 198, 198,   0,   0,   0,   0,  48,  48,   0,  56,  48,  48,  48,  48,  48, 120,
	  0,   0,   0,  96,  96,   0, 112,  96,  96,  96,  96,  96,  96, 102, 102,  60,   0,   0,   6,   6,   6, 198, 102,  54,  30,  54, 102, 198,   0,   0,
	  0,   0,  56,  48,  48,  48,  48,  48,  48,  48,  48, 120,   0,   0,   0,   0,   0,   0,   0, 126, 214, 214, 214, 214, 214, 214,   0,   0,   0,   0,
	  0,   0,   0, 126, 198, 198, 198, 198, 198, 198,   0,   0,   0,   0,   0,   0,   0, 124, 198, 198, 198, 198, 198, 124,   0,   0,   0,   0,   0,   0,
	  0, 126, 198, 198, 198, 198, 198, 126,   6,   6,   0,   0,   0,   0,   0, 252, 198, 198, 198, 198, 198, 252, 192, 192,   0,   0,   0,   0,   0, 246,
	 30,  14,   6,   6,   6,   6,   0,   0,   0,   0,   0,   0,   0, 252,   6,   6, 124, 192, 192, 126,   0,   0,   0,   0,  24,  24,  24, 126,  24,  24,
	 24,  24,  24, 240,   0,   0,   0,   0,   0,   0,   0, 198, 198, 198, 198, 198, 198, 252,   0,   0,   0,   0,   0,   0,   0, 198, 198, 198, 108, 108,
	 56,  56,   0,   0,   0,   0,   0,   0,   0, 198, 198, 214, 214, 214, 214, 124,   0,   0,   0,   0,   0,   0,   0, 198, 198, 108,  56, 108, 198, 198,
	  0,   0,   0,   0,   0,   0,   0, 198, 198, 198, 198, 198, 198, 252, 192, 192,   0,   0,   0,   0,   0, 254,  96,  48,  24,  12,   6, 254,   0,   0,
	  0,   0, 112,  24,  24,  24,  12,  24,  24,  24,  24, 112,   0,   0,   0,   0,  48,  48,  48,  48,  48,  48,  48,  48,  48,  48,   0,   0,   0,   0,
	 28,  48,  48,  48,  96,  48,  48,  48,  48,  28,   0,   0,   0,   0,   0, 206, 219, 115,   0,   0,   0,   0,   0,   0,   0,   0,
};


void Pixel_Canvas::draw_text(const std::string& text, v2i coord, u32 color)
{
	i32 y = coord.y;
	i32 x = coord.x;

	for (i32 c = 0; c < text.length(); c++)
	{
		if (text[c] == '\n')
		{
			y += s_build_in_font_char_height;
			x = coord.x;
			continue;
		}

		i32 character_idx = text[c] - 33;
		if (character_idx < 0 || character_idx > 93)
			goto INCREMENT;
		
		character_idx *= s_build_in_font_char_height;
		
		for (i32 row = 0; row < s_build_in_font_char_height; row++)
		{
			for (i32 bit = 0; bit < s_build_in_font_char_width; bit++)
			{
				//i32 x = coord.x + c * s_build_in_font_char_width + bit;
				if (s_build_in_font[character_idx + row] & (1 << bit))
					set_pixel({ x + bit,y + row}, color);
			}
		}

		INCREMENT:
		x += s_build_in_font_char_width;
	}
}