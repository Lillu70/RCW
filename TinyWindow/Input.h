#pragma once
#include "Primitives.h"

#include <cmath>

enum class Key_Code
{
	NUM_0 = 0,
	NUM_1,
	NUM_2,
	NUM_3,
	NUM_4,
	NUM_5,
	NUM_6,
	NUM_7,
	NUM_8,
	NUM_9,
	NP_NUM_0,
	NP_NUM_1,
	NP_NUM_2,
	NP_NUM_3,
	NP_NUM_4,
	NP_NUM_5,
	NP_NUM_6,
	NP_NUM_7,
	NP_NUM_8,
	NP_NUM_9,
	NP_DIVIDE,
	NP_MULTIPLY,
	NP_ADD,
	NP_SUBTRACT,
	NP_DECIMAL,
	A,
	B,
	C,
	D,
	E,
	F,
	G,
	H,
	I,
	J,
	K,
	L,
	M,
	N,
	O,
	P,
	Q,
	R,
	S,
	T,
	U,
	V,
	W,
	X,
	Y,
	Z,
	F1,
	F2,
	F3,
	F4,
	F5,
	F6,
	F7,
	F8,
	F9,
	F10,
	F11,
	F12,
	LEFT,
	RIGHT,
	UP,
	DOWN,
	ESC,
	LCTRL,
	LSHIFT,
	LALT,
	LSYS,
	LMOUSE,
	RCTRL,
	RSHIFT,
	RALT,
	RSYS,
	RMOUSE,
	SPACE,
	PAGEUP,
	PAGEDOWN,
	END,
	HOME,
	INSERT,
	DELETE,
	BACK,
	ENTER,
	KEY_CODE_COUNT,
};

inline Key_Code& operator++(Key_Code& code) { *((i32*)&code) += 1; return code; }

enum class Button
{
	DPAD_UP = 0, 
	DPAD_DOWN,
	DPAD_LEFT,
	DPAD_RIGHT,
	START,
	BACK,
	L_THUMB,
	R_THUMB,
	L_SHLD,
	R_SHLD,
	BUT_A,
	BUT_B,
	BUT_X,
	BUT_Y, 
	BUTTON_COUNT
};

struct Button_State
{
	Button_State(bool curr, bool prev) : m_curr(curr), m_prev(prev) {};

	bool is_down() { return m_curr; }
	bool is_up() { return !m_curr; }
	bool is_pressed() { return !m_prev && m_curr; }
	bool is_released() { return m_prev && !m_curr; }

private:
	bool m_curr;
	bool m_prev;
};

struct Controller_State
{
	struct Data
	{
		u16 button_states = 0;
		f32 l_thumb_x = 0;
		f32 l_thumb_y = 0;
		f32 r_thumb_x = 0;
		f32 r_thumb_y = 0;
		f32 l_trig = 0;
		f32 r_trig = 0;
	};

	Controller_State::Data m_curr;
	Controller_State::Data m_prev;

	Button_State get_button_state(Button button)
	{
		i16 b = (i16)button;
		return Button_State((m_curr.button_states & (1 << (int)b)) > 0, (m_prev.button_states & (1 << (int)b)) > 0);
	}

	bool get_button_down(Button button)
	{
		return (m_curr.button_states & (1 << (int)button)) > 0;
	}

	static constexpr f32 s_default_dead_zone = 0.25f;

	bool get_left_stick_x(f32& output, f32 dead_zone = s_default_dead_zone)
	{
		if (std::abs(m_curr.l_thumb_x) < dead_zone) return false;
		if (m_curr.l_thumb_x > 0)
			output = (m_curr.l_thumb_x - dead_zone) / (1.f - dead_zone);
		else
			output = (m_curr.l_thumb_x + dead_zone) / (-1.f + dead_zone) * -1;
		
		return true;
	}

	bool get_left_stick_y(f32& output, f32 dead_zone = s_default_dead_zone)
	{
		if (std::abs(m_curr.l_thumb_y) < dead_zone) return false;
		if (m_curr.l_thumb_y > 0)
			output = (m_curr.l_thumb_y - dead_zone) / (1.f - dead_zone);
		else
			output = (m_curr.l_thumb_y + dead_zone) / (-1.f + dead_zone) * -1;
		return true;
	}

	bool get_right_stick_x(f32& output, f32 dead_zone = s_default_dead_zone)
	{
		if (std::abs(m_curr.r_thumb_x) < dead_zone) return false;
		if (m_curr.r_thumb_x > 0)
			output = (m_curr.r_thumb_x - dead_zone) / (1.f - dead_zone);
		else
			output = (m_curr.r_thumb_x + dead_zone) / (-1.f + dead_zone) * -1;
		return true;
	}

	bool get_right_stick_y(f32& output, f32 dead_zone = s_default_dead_zone)
	{
		if (std::abs(m_curr.r_thumb_y) < dead_zone) return false;
		if (m_curr.r_thumb_y > 0)
			output = (m_curr.r_thumb_y - dead_zone) / (1.f - dead_zone);
		else
			output = (m_curr.r_thumb_y + dead_zone) / (-1.f + dead_zone) * -1;
		return true;
	}
};