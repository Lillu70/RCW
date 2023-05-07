#pragma once

#include "../TW_PLATFORM_INTERFACE.h"
#include "../Memory_Arena.h"

#define NOMINMAX
#include <Windows.h>
#include "TW_Sound.h"
#include <string>


#define KiB 1024
#define MiB KiB * KiB
#define GAME_STATE_MEMORY_SIZE MiB
#define PLATFORM_STATE_MEMORY_SIZE MiB / 2

struct TW_App_Config
{
	std::string title = "Tiny Windows App";

	i32 window_position_x = CW_USEDEFAULT;
	i32 window_position_y = CW_USEDEFAULT;

	i32 window_width = CW_USEDEFAULT;
	i32 window_height = CW_USEDEFAULT;
};

static TW_Platform_Call_Table win32_get_call_table();

static void win32_init(HINSTANCE instance, TW_App_Config config);
static void win32_flush_events();
static void win32_update_surface();

static void* win32_read_entire_file(const char* file_path);
static void win32_free_file_memory(void* memory);
static bool win32_write_entire_file(const char* file_path, u32 file_size, void* memory);

static u32* win32_do_resize_pixel_buffer(i32 new_width, i32 new_height);
static void win32_do_close();

static bool win32_get_is_running();
static bool win32_get_is_focused();
static u32* win32_get_pixel_buffer();
static i32	win32_get_pixel_buffer_width();
static i32	win32_get_pixel_buffer_height();
static f32	win32_get_frame_time();
static i32	win32_get_frames_per_second();
static u64	win32_get_cycles_per_second();
static i64	win32_get_cpu_time_stamp();
static bool win32_get_is_fullscreen();
static bool win32_get_keyboard_button_down(Key_Code key_code);
static Button_State win32_get_keyboard_state(Key_Code key_code);
static Controller_State win32_get_controller_state(i32 idx);
static u32 win32_get_window_width();
static u32 win32_get_window_height();
static char* win32_get_cwd();

static void win32_output_debug_string(const char* str);

static void win32_set_fullscreen(bool enalbed);

static constexpr i32 s_controller_map[(u64)Button::BUTTON_COUNT] =
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