#pragma once

#include "../TW_PLATFORM_INTERFACE.h"

#define NOMINMAX
#include <Windows.h>

#include "TW_Sound.h"

#include <string>


struct TW_App_Config
{
	std::string title = "Tiny Windows App";

	i32 window_position_x = CW_USEDEFAULT;
	i32 window_position_y = CW_USEDEFAULT;

	i32 window_width = CW_USEDEFAULT;
	i32 window_height = CW_USEDEFAULT;
};

class TW_App : public TW_Platform_Interface
{
	//TODO(Azenor): Insert debug assertions.

public:
	TW_App(HINSTANCE instance, TW_App_Config config);
	
	void flush_events();
	
	void update_surface();

public: //virtual interface to be passed into a client layer.
	u32* do_resize_pixel_buffer(i32 new_width, i32 new_height) override;

	i64 get_cpu_time_stamp() override;

	void do_close()					override	{ m_app_running = false;		}
	bool get_is_running()			override	{ return m_app_running;			}
	bool get_is_focused()			override	{ return m_is_focused;			}
	u32* get_pixel_buffer()			override	{ return m_bitmap_mem;			}
	i32 get_pixel_buffer_width() 	override	{ return m_bitmap_width;		}
	i32 get_pixel_buffer_height() 	override	{ return m_bitmap_height;		}
	f32 get_frame_time()			override	{ return m_frame_time;			}
	i32 get_frames_per_second()		override	{ return m_frames_per_second;	}
	u64 get_cycles_per_second()		override	{ return m_cycles_per_second;	}
	
	Controller_State get_controller_state(i32 idx) override;
	Button_State get_keyboard_state(Key_Code key_code) override;
	bool get_keyboard_button_down(Key_Code key_code) override;

public:
	bool m_include_perf_metrics_in_title = true;

public:
	static constexpr i32 s_max_controllers = 1;

private:
	void init_bitmap_info();

	void create_window(TW_App_Config& config);

	void init_time_counter();

	void update_timer_counter();

	void update_title_perf_info();

	bool load_XInput();

	void update_controller_state();

private:
	static LRESULT CALLBACK windows_callback(HWND win_handle, UINT message, WPARAM wparam, LPARAM lParam);

private:
	std::string m_title;

	static TW_App* s_app_instance;
	static constexpr i32 s_bytes_per_pixel = 4;

	bool m_app_running = true;
	bool m_is_focused = false;
	
	
	Controller_State m_controller_state[s_max_controllers];
	bool m_keyboard_state_a[(u64)Key_Code::KEY_CODE_COUNT]{};
	bool m_keyboard_state_b[(u64)Key_Code::KEY_CODE_COUNT]{};
	bool* m_curr_keyboard_state = m_keyboard_state_a;
	bool* m_prev_keyboard_state = m_keyboard_state_b;
	

	BITMAPINFO m_bitmap_info{};
	i32 m_bitmap_width = 0, m_bitmap_height = 0;
	u32* m_bitmap_mem = nullptr;

	HDC m_device_context = nullptr;
	HINSTANCE m_instance = nullptr;
	HWND m_window = nullptr;

	i32 m_window_width = 0, m_window_height = 0;

	i64 m_timer_counter_freg = 0;
	LARGE_INTEGER m_last_time_counter;
	f32 m_frame_time = 0;
	f32 m_accum_frame_time = 0;
	i32 m_frame_counter = 1;
	i32 m_frames_per_second = 0;
	u64 m_frame_start_cpu_stamp = 0;
	u64 m_accum_cpu_cycles = 0;
	u64 m_cycles_per_second = 0;

	TW_Sound m_sound;
};


static constexpr i16 s_controller_map[(u64)Button::BUTTON_COUNT] =
{
	0x0001,	//0
	0x0002,	//1
	0x0004,	//2
	0x0008,	//3
	0x0010,	//4
	0x0020,	//5
	0x0040,	//6
	0x0080,	//7
	0x0100,	//8
	0x0200,	//9
	0x1000,	//10
	0x2000,	//11
	0x4000,	//12
	0x8000  //13
};

static constexpr i32 s_keycode_map[(u64)Key_Code::KEY_CODE_COUNT] = 
{
	'0',
	'1',
	'2',
	'3',
	'4',
	'5',
	'6',
	'7',
	'8',
	'9',
	VK_NUMPAD0,
	VK_NUMPAD1,
	VK_NUMPAD2,
	VK_NUMPAD3,
	VK_NUMPAD4,
	VK_NUMPAD5,
	VK_NUMPAD6,
	VK_NUMPAD7,
	VK_NUMPAD8,
	VK_NUMPAD9,
	VK_DIVIDE,
	VK_MULTIPLY,
	VK_ADD,
	VK_SUBTRACT,
	VK_DECIMAL,
	'A',
	'B',
	'C',
	'D',
	'E',
	'F',
	'G',
	'H',
	'I',
	'J',
	'K',
	'L',
	'M',
	'N',
	'O',
	'P',
	'Q',
	'R',
	'S',
	'T',
	'U',
	'V',
	'W',
	'X',
	'Y',
	'Z',
	VK_F1,
	VK_F2,
	VK_F3,
	VK_F4,
	VK_F5,
	VK_F6,
	VK_F7,
	VK_F8,
	VK_F9,
	VK_F10,
	VK_F11,
	VK_F12,
	VK_LEFT,
	VK_RIGHT,
	VK_UP,
	VK_DOWN,
	VK_ESCAPE,
	VK_LCONTROL,
	VK_LSHIFT,
	VK_LMENU,
	VK_LWIN,
	VK_LBUTTON,
	VK_RCONTROL,
	VK_RSHIFT,
	VK_RMENU,
	VK_RWIN,
	VK_RBUTTON,
	VK_SPACE,
	VK_PRIOR,
	VK_NEXT,
	VK_END,
	VK_HOME,
	VK_INSERT,
	VK_DELETE,
	VK_BACK,
	VK_RETURN
};