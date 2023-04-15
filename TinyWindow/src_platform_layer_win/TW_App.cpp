#include "TW_App.h"
#include <Xinput.h>


#define TW_TERMINATE(X)										\
{															\
	OutputDebugStringA((std::string(X) + "\n").c_str());	\
	abort();												\
}

#ifdef _DEBUG
#define TW_ASSERT(X, MESSAGE)		\
if(!(X))							\
{									\
	OutputDebugStringA(MESSAGE);	\
	abort();						\
}								
#else
#define TW_ASSERT(X, MESSAGE);
#endif

typedef DWORD WINAPI x_input_get_state(DWORD dwUserIndex, XINPUT_STATE* state);
static x_input_get_state* XInputGetState_;
#define XInputGetState XInputGetState_

typedef DWORD WINAPI x_input_set_state(DWORD dwUserIndex, XINPUT_VIBRATION* vibrarion);
static x_input_set_state* XInputSetState_;
#define XInputSetState XInputSetState_

struct WIN32_CORE
{
	HDC dc = nullptr;
	HINSTANCE instance = nullptr;
	HWND window = nullptr;
	void* game_state_memory = nullptr;
};
static WIN32_CORE s_core;

struct WIN32_BITMAP
{
	inline static constexpr i32 bytes_per_pixel = 4;

	BITMAPINFO info{};
	i32 width = 0, height = 0;
	u32* memory = nullptr;
};
static WIN32_BITMAP s_bitmap;

struct WIN32_APP
{
	std::string title;

	WINDOWPLACEMENT window_placement = { sizeof(window_placement) };
	i32 window_width = 0, window_height = 0;
	bool is_running = true;
	bool is_focused = false;
	bool is_fullscreen = false;
	bool include_perf_metrics_in_title = true;
};
static WIN32_APP s_app;

struct WIN32_INPUT
{
	inline static constexpr i32 max_controllers = 1;

	Controller_State controller_state[max_controllers];
	bool keyboard_state_a[(u64)Key_Code::KEY_CODE_COUNT]{};
	bool keyboard_state_b[(u64)Key_Code::KEY_CODE_COUNT]{};
	bool* curr_keyboard_state = keyboard_state_a;
	bool* prev_keyboard_state = keyboard_state_b;
};
static WIN32_INPUT s_input;

struct WIN32_TIME
{
	i64 timer_counter_freg = 0;
	LARGE_INTEGER last_time_counter;
	f32 frame_time = 0;
	f32 accum_frame_time = 0;
	i32 frame_counter = 1;
	i32 frames_per_second = 0;
	u64 frame_start_cpu_stamp = 0;
	u64 accum_cpu_cycles = 0;
	u64 cycles_per_second = 0;
};
static WIN32_TIME s_time;


static LRESULT tw_windows_callback(HWND win_handle, UINT message, WPARAM wparam, LPARAM lParam);
static inline void update_controller_state();

TW_Platform_Call_Table win32_get_call_table()
{
	TW_Platform_Call_Table ct;

	ct.do_close						= win32_do_close;
	ct.do_resize_pixel_buffer		= win32_do_resize_pixel_buffer;
	ct.get_controller_state			= win32_get_controller_state;
	ct.get_cpu_time_stamp			= win32_get_cpu_time_stamp;
	ct.get_cycles_per_second		= win32_get_cycles_per_second;
	ct.get_frames_per_second		= win32_get_frames_per_second;
	ct.get_frame_time				= win32_get_frame_time;
	ct.get_is_focused				= win32_get_is_focused;
	ct.get_is_fullscreen			= win32_get_is_fullscreen;
	ct.get_is_running				= win32_get_is_running;
	ct.get_keyboard_button_down		= win32_get_keyboard_button_down;
	ct.get_keyboard_state			= win32_get_keyboard_state;
	ct.get_pixel_buffer				= win32_get_pixel_buffer;
	ct.get_pixel_buffer_height		= win32_get_pixel_buffer_height;
	ct.get_pixel_buffer_width		= win32_get_pixel_buffer_width;
	ct.set_fullscreen				= win32_set_fullscreen;
	ct.output_debug_string			= win32_output_debug_string;

	return ct;
}

void win32_init(HINSTANCE instance, TW_App_Config config)
{
	if (s_core.instance != nullptr)
		TW_TERMINATE("App already initialized");

	s_core.instance = instance;
	s_app.title = config.title;
	
	//Create window
	{
		WNDCLASSA win_class{};
		//window_class.hIcon = ;
		win_class.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
		win_class.lpfnWndProc = tw_windows_callback;
		win_class.hInstance = s_core.instance;
		win_class.lpszClassName = "TinyWindow_Class";

		if (!RegisterClassA(&win_class))
			TW_TERMINATE("Registering the window class failed.");

		//Windows behavior here is whacky.
		//https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createwindowexa
		if (config.window_position_x != CW_USEDEFAULT && config.window_position_y == CW_USEDEFAULT) config.window_position_y = 0;
		if (config.window_position_y != CW_USEDEFAULT && config.window_position_x == CW_USEDEFAULT) config.window_position_x = 0;

		if (config.window_height != CW_USEDEFAULT && config.window_width == CW_USEDEFAULT) config.window_width = config.window_height;
		if (config.window_width != CW_USEDEFAULT && config.window_height == CW_USEDEFAULT) config.window_height = config.window_width;

		DWORD win_style_flags = WS_OVERLAPPEDWINDOW | WS_VISIBLE;

		s_core.window = CreateWindowExA(0, win_class.lpszClassName, config.title.c_str(),
			win_style_flags, config.window_position_x, config.window_position_y, config.window_width, config.window_height, 0, 0, s_core.instance, 0);

		if (!s_core.window) TW_TERMINATE("Creating the window failed.");
		
		s_core.dc = GetDC(s_core.window);
	}
	
	s_bitmap.info.bmiHeader.biSize = sizeof(s_bitmap.info.bmiHeader);
	s_bitmap.info.bmiHeader.biPlanes = 1;
	s_bitmap.info.bmiHeader.biBitCount = sizeof(i32) * 8;
	s_bitmap.info.bmiHeader.biCompression = BI_RGB;

	//LOAD XInput
	{ 

		HMODULE XInput_lib;
		if ((XInput_lib = LoadLibraryA("xinput1_4.dll")) || (XInput_lib = LoadLibraryA("xinput1_3.dll")))
		{
			XInputGetState = (x_input_get_state*)GetProcAddress(XInput_lib, "XInputGetState");
			XInputSetState = (x_input_set_state*)GetProcAddress(XInput_lib, "XInputSetState");
		}
	}
	
	
	{ //Init time counters
		LARGE_INTEGER perf_freg;
		QueryPerformanceFrequency(&perf_freg);
		s_time.timer_counter_freg = perf_freg.QuadPart;

		QueryPerformanceCounter(&s_time.last_time_counter);

		s_time.frame_start_cpu_stamp = __rdtsc();
	}

	s_core.game_state_memory = VirtualAlloc(0, GAME_STATE_MEMORY_SIZE, MEM_COMMIT, PAGE_READWRITE);
}

void win32_flush_events()
{
	MSG message;
	while (BOOL message_result = PeekMessage(&message, 0, 0, 0, PM_REMOVE) != 0)
	{
		if (message_result == -1) TW_TERMINATE("GetMessage encountered an error.");

		TranslateMessage(&message);
		DispatchMessageA(&message);
	}

	update_controller_state();
	
	std::swap(s_input.curr_keyboard_state, s_input.prev_keyboard_state);
	
	for (u32 i = 0; i < (u32)Key_Code::KEY_CODE_COUNT; ++i)
		s_input.curr_keyboard_state[i] = (GetKeyState(s_keycode_map[i]) & (1 << 15)) > 0;
	
	//Update timing info.
	{
		LARGE_INTEGER current_time;
		QueryPerformanceCounter(&current_time);

		i64 counter_elapsed_time = current_time.QuadPart - s_time.last_time_counter.QuadPart;
		s_time.frame_time = f32(counter_elapsed_time) / s_time.timer_counter_freg;
		s_time.last_time_counter = current_time;

		u64 frame_end_stamp = __rdtsc();
		u64 frame_cycle_count = frame_end_stamp - s_time.frame_start_cpu_stamp;
		s_time.frame_start_cpu_stamp = frame_end_stamp;
		s_time.accum_cpu_cycles += frame_cycle_count;

		s_time.accum_frame_time += s_time.frame_time;
		if (s_time.accum_frame_time >= 1.f)
		{
			--s_time.accum_frame_time;
			s_time.cycles_per_second = s_time.accum_cpu_cycles / s_time.frame_counter;
			s_time.accum_cpu_cycles = 0;
			s_time.frames_per_second = s_time.frame_counter;
			s_time.frame_counter = 0;

			if (s_app.include_perf_metrics_in_title)
			{
				std::string title = s_app.title + " --[FPS:" + std::to_string(s_time.frames_per_second) + "] -- [KCPS:" + std::to_string(s_time.cycles_per_second / 1000) + "]";
				SetWindowTextA(s_core.window, title.c_str());
			}
		}

		++s_time.frame_counter;
	}
}

void win32_update_surface()
{
	if(s_bitmap.memory)
		StretchDIBits(s_core.dc, 0, 0, s_app.window_width, s_app.window_height, 0, 0, s_bitmap.width, s_bitmap.height, s_bitmap.memory, &s_bitmap.info, DIB_RGB_COLORS, SRCCOPY);
}

static inline void update_controller_state()
{
	for (i32 i = 0; i < s_input.max_controllers; i++)
	{
		s_input.controller_state[i].m_prev = s_input.controller_state[i].m_curr;

		if (XInputGetState)
		{
			XINPUT_STATE xinput_state;
			if (XInputGetState(i, &xinput_state) == ERROR_SUCCESS)
			{
				auto& pad = xinput_state.Gamepad;
				s_input.controller_state[i].m_curr.button_states = 0;
				for (u16 button_idx = 0; button_idx < (u16)Button::BUTTON_COUNT; ++button_idx)
					if (s_controller_map[button_idx] & pad.wButtons)
						s_input.controller_state[i].m_curr.button_states = s_input.controller_state[i].m_curr.button_states | (1 << button_idx);

				static constexpr u32 negative_max_range = 32768;
				static constexpr u32 positive_max_range = 32767;

				if (pad.sThumbLX < 0)
					s_input.controller_state[i].m_curr.l_thumb_x = (f32)pad.sThumbLX / negative_max_range;
				else
					s_input.controller_state[i].m_curr.l_thumb_x = (f32)pad.sThumbLX / positive_max_range;

				if (pad.sThumbLY < 0)
					s_input.controller_state[i].m_curr.l_thumb_y = (f32)pad.sThumbLY / negative_max_range;
				else
					s_input.controller_state[i].m_curr.l_thumb_y = (f32)pad.sThumbLY / positive_max_range;

				if (pad.sThumbRX < 0)
					s_input.controller_state[i].m_curr.r_thumb_x = (f32)pad.sThumbRX / negative_max_range;
				else
					s_input.controller_state[i].m_curr.r_thumb_x = (f32)pad.sThumbRX / positive_max_range;

				if (pad.sThumbRY < 0)
					s_input.controller_state[i].m_curr.r_thumb_y = (f32)pad.sThumbRY / negative_max_range;
				else
					s_input.controller_state[i].m_curr.r_thumb_y = (f32)pad.sThumbRY / positive_max_range;

				static constexpr u32 trigger_max_range = 255;

				s_input.controller_state[i].m_curr.l_trig = (f32)pad.bLeftTrigger / trigger_max_range;
				s_input.controller_state[i].m_curr.r_trig = (f32)pad.bRightTrigger / trigger_max_range;
			}
		}
	}
}


static LRESULT tw_windows_callback(HWND win_handle, UINT message, WPARAM wparam, LPARAM lParam)
{
	LRESULT result = 0;

	switch (message)
	{
	case WM_SIZE:
	{
		RECT client_rect;
		GetClientRect(win_handle, &client_rect);

		s_app.window_width = client_rect.right - client_rect.left;
		s_app.window_height = client_rect.bottom - client_rect.top;
	}break;

	case WM_DESTROY:
	{
		s_app.is_running = false;
	}break;

	case WM_CLOSE:
	{
		s_app.is_running = false;
	}break;

	case WM_ACTIVATEAPP:
	{
		s_app.is_focused = !s_app.is_focused;
	}break;

	default:
	{
		result = DefWindowProcA(win_handle, message, wparam, lParam);
	}break;
	}

	return  result;
}

u32* win32_do_resize_pixel_buffer(i32 new_width, i32 new_height)
{
	TW_ASSERT(new_width >= 0, "Pixel buffer width has to be a value greater than 0");
	TW_ASSERT(new_height >= 0, "Pixel buffer hegith has to be a value greater than 0");

	s_bitmap.height = new_height;
	s_bitmap.width = new_width;

	if (s_bitmap.memory)
		delete[] s_bitmap.memory;

	s_bitmap.info.bmiHeader.biWidth = new_width;
	s_bitmap.info.bmiHeader.biHeight = new_height * -1;
	//^ windows defaults to drawring from bottom left, 
	//setting the bit map height to negative tells it to draw from top left instead.

	u64 pixel_area = (u64)new_width * new_height;
	u64 bitmap_memory_reg = pixel_area * s_bitmap.bytes_per_pixel;

	s_bitmap.memory = new u32[pixel_area];

	return s_bitmap.memory;
}

Controller_State win32_get_controller_state(i32 idx)
{
	TW_ASSERT(idx >= 0 && idx < s_input.max_controllers, "Controller index array out of bounds!");
	return s_input.controller_state[idx];
}

void win32_output_debug_string(const char* str)
{
	OutputDebugStringA(str);
}

void win32_set_fullscreen(bool enalbed)
{
	if (s_app.is_fullscreen == enalbed) return;

	DWORD dwStyle = GetWindowLongA(s_core.window, GWL_STYLE);

	if (enalbed)
	{
		//TODO: Log errors.

		if (!GetWindowPlacement(s_core.window, &s_app.window_placement))
			return;

		MONITORINFO mi = { sizeof(mi) };

		if (!GetMonitorInfoA(MonitorFromWindow(s_core.window, MONITOR_DEFAULTTOPRIMARY), &mi))
			return;

		SetWindowLongA(s_core.window, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);
		SetWindowPos(s_core.window, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top, mi.rcMonitor.right - mi.rcMonitor.left, mi.rcMonitor.bottom - mi.rcMonitor.top, SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
	}
	else
	{
		SetWindowLongA(s_core.window, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(s_core.window, &s_app.window_placement);
		SetWindowPos(s_core.window, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
	}

	s_app.is_fullscreen = enalbed;
}

void win32_do_close() { s_app.is_running = false; }

bool win32_get_is_running() { return s_app.is_running; }

bool win32_get_is_focused() { return s_app.is_focused; }

u32* win32_get_pixel_buffer() { return s_bitmap.memory; }

i32 win32_get_pixel_buffer_width() { return s_bitmap.width; }

i32 win32_get_pixel_buffer_height() { return s_bitmap.height; }

f32 win32_get_frame_time() { return s_time.frame_time; }

i32 win32_get_frames_per_second() { return s_time.frames_per_second; }

u64 win32_get_cycles_per_second() { return s_time.cycles_per_second; }

i64 win32_get_cpu_time_stamp() { return __rdtsc(); }

bool win32_get_is_fullscreen() { return s_app.is_fullscreen; }

bool win32_get_keyboard_button_down(Key_Code key_code) { return s_input.curr_keyboard_state[(i32)key_code]; }

Button_State win32_get_keyboard_state(Key_Code key_code)
{
	return Button_State(s_input.curr_keyboard_state[(i32)key_code], s_input.prev_keyboard_state[(i32)key_code]);
}