#pragma once

#include "Input.h"


struct TW_Platform_Call_Table
{
	u32* (*do_resize_pixel_buffer)(i32, i32)		= 0;
	void (*do_close)()								= 0;
	i64  (*get_cpu_time_stamp)()					= 0;
	bool (*get_is_running)()						= 0;
	bool (*get_is_focused)()						= 0;
	u32* (*get_pixel_buffer)()						= 0;
	i32  (*get_pixel_buffer_width)()				= 0;
	i32	 (*get_pixel_buffer_height)()				= 0;
	f32  (*get_frame_time)()						= 0;
	i32  (*get_frames_per_second)()					= 0;
	u64  (*get_cycles_per_second)()					= 0;
	bool (*get_keyboard_button_down)(Key_Code)		= 0;
	bool (*get_is_fullscreen)()						= 0;
	void (*set_fullscreen)(bool)					= 0;
	void (*output_debug_string)(const char*)		= 0;

	Button_State(*get_keyboard_state)(Key_Code)		= 0;
	Controller_State(*get_controller_state)(i32)	= 0;
};