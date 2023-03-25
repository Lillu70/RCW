#pragma once

#include "Input.h"

class TW_Platform_Interface
{
public:
	virtual u32* resize_pixel_buffer(i32 new_width, i32 new_height)		= 0;
	virtual i64 get_cpu_time_stamp()									= 0;
	virtual Controller_State get_controller_state(i32 idx = 0)			= 0;
	virtual bool is_running()											= 0;
	virtual bool is_focused()											= 0;
	virtual u32* get_pixel_buffer()										= 0;
	virtual i32 get_pixel_buffer_width()								= 0;
	virtual i32 get_pixel_buffer_height()								= 0;
	virtual f32 get_frame_time()										= 0;
	virtual i32 get_frames_per_second()									= 0;
	virtual u64 get_cycles_per_second()									= 0;
	virtual void close()												= 0;
	virtual Button_State get_keyboard_state(Key_Code key_code)			= 0;
	virtual bool get_keyboard_button_down(Key_Code key_code)			= 0;
};
