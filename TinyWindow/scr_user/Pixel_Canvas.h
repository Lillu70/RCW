#pragma once
#include "Types.h"
#include "Maths.h"

#include <string>

class Pixel_Canvas final
{
public:
	static constexpr int s_build_in_font_char_width = 8;
	static constexpr int s_build_in_font_char_height = 14;

	Pixel_Canvas(u32* pixels, v2u dimensions);
	~Pixel_Canvas();
	
	v2u dimensions()	{ return m_dimensions;		}
	u32 width()			{ return m_dimensions.x;	}
	u32 height()		{ return m_dimensions.y;	}

	void clear_random();
	void clear(u32 color);
	inline void set_pixel(v2i coord, u32 color);

	void draw_filled_circle(v2i center, u32 radius, u32 color);
	void draw_circle(v2i center, u32 radius, u32 thickness, u32 color);
	void draw_line(v2i p1, v2i p2, u32 color);
	void draw_filled_rect(v2i p1, v2i p2, u32 color);
	void draw_rect(v2i p1, v2i p2, i32 thickeness, u32 color);
	void draw_vertical_column(i32 x, i32 y_top, i32 y_bot, u32 color);
	void draw_text(const std::string& text, v2i coord, u32 color);
	
	bool is_on_canvas(v2i coord);

private:
	inline bool is_on_canvas(u32 pixel_idx);
	inline u32 coord_to_idx(v2u coord);
	
private:
	u32* m_pixels = nullptr;
	v2u m_dimensions;
};

inline u32 put_color(u8 r, u8 g, u8 b, u8 a = 0xff)
{
	return u32(a << 24) | u32(r << 16) | u32(g << 8) | u32(b << 0);
}

