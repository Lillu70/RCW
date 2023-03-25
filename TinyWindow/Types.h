#pragma once

#include <cmath>
#include <inttypes.h>


typedef uint8_t		u8;
typedef uint16_t	u16;
typedef uint32_t	u32;
typedef uint64_t	u64;

typedef int8_t		i8;
typedef int16_t		i16;
typedef int32_t		i32;
typedef int64_t		i64;

typedef float		f32;
typedef double		f64;

typedef i32			b32;

enum class Key_Codes
{
	W = 0,
	A,
	S,
	D,
	LEFT,
	RIGHT,
	ESC,
	KEY_CODE_COUNT,
};

inline Key_Codes& operator++(Key_Codes& code) { *((i32*)&code) += 1; return code; }

enum class Buttons
{
	DPAD_UP = 0x0001,
	DPAD_DOWN = 0x0002,
	DPAD_LEFT = 0x0004,
	DPAD_RIGHT = 0x0008,
	START = 0x0010,
	BACK = 0x0020,
	L_THUMB = 0x0040,
	R_THUMB = 0x0080,
	L_SHLD = 0x0100,
	R_SHLD = 0x0200,
	BUT_A = 0x1000,
	BUT_B = 0x2000,
	BUT_X = 0x4000,
	BUT_Y = 0x8000,
};

struct Button_State
{
	Button_State(bool curr, bool prev) : m_curr(curr), m_prev(prev) {};

	bool is_down()		{ return m_curr;			}
	bool is_up()		{ return !m_curr;			}
	bool is_pressed()	{ return !m_prev && m_curr; }
	bool is_released()	{ return m_prev && !m_curr; }

private:
	bool m_curr;
	bool m_prev;
};

struct Controller_State
{
	struct Data
	{
		u16 button_states	= 0;
		f32 l_thumb_x		= 0;
		f32 l_thumb_y		= 0;
		f32 r_thumb_x		= 0;
		f32 r_thumb_y		= 0;
		f32 l_trig			= 0;
		f32 r_trig			= 0;
	};

	Controller_State::Data m_curr;
	Controller_State::Data m_prev;

	Button_State get(Buttons button)
	{
		i16 b = (i16)button;
		return Button_State((m_curr.button_states & b) > 0, (m_prev.button_states & b) > 0);
	}

	static constexpr f32 s_default_dead_zone = 0.3f;

	bool get_left_stick_x(f32& output, f32 dead_zone = s_default_dead_zone)
	{
		if (std::abs(m_curr.l_thumb_x) < dead_zone) return false;
		output = m_curr.l_thumb_x;
		return true;
	}

	bool get_left_stick_y(f32& output, f32 dead_zone = s_default_dead_zone)
	{
		if (std::abs(m_curr.l_thumb_y) < dead_zone) return false;
		output = m_curr.l_thumb_y;
		return true;
	}

	bool get_right_stick_x(f32& output, f32 dead_zone = s_default_dead_zone)
	{
		if (std::abs(m_curr.r_thumb_x) < dead_zone) return false;
		output = m_curr.r_thumb_x;
		return true;
	}

	bool get_right_stick_y(f32& output, f32 dead_zone = s_default_dead_zone)
	{
		if (std::abs(m_curr.r_thumb_y) < dead_zone) return false;
		output = m_curr.r_thumb_y;
		return true;
	}
};