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


TW_App* TW_App::s_app_instance = nullptr;

typedef DWORD WINAPI x_input_get_state(DWORD dwUserIndex, XINPUT_STATE* state);
static x_input_get_state* XInputGetState_;
#define XInputGetState XInputGetState_

typedef DWORD WINAPI x_input_set_state(DWORD dwUserIndex, XINPUT_VIBRATION* vibrarion);
static x_input_set_state* XInputSetState_;
#define XInputSetState XInputSetState_

TW_App::TW_App(HINSTANCE instance, TW_App_Config config)
{
	if (s_app_instance)
		TW_TERMINATE("Multiple instances of the app created.");

	s_app_instance = this;

	m_title = config.title;
	m_instance = instance;
	create_window(config);

	m_device_context = GetDC(m_window);

	init_bitmap_info();

	load_XInput();
	
	init_time_counter();
}

void TW_App::create_window(TW_App_Config& config)
{
	WNDCLASSA win_class{};
	//window_class.hIcon = ;
	win_class.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	win_class.lpfnWndProc = windows_callback;
	win_class.hInstance = m_instance;
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

	m_window = CreateWindowExA(0, win_class.lpszClassName, config.title.c_str(),
		win_style_flags, config.window_position_x, config.window_position_y, config.window_width, config.window_height, 0, 0, m_instance, 0);

	if (!m_window) TW_TERMINATE("Creating the window failed.");
}

void TW_App::init_time_counter()
{
	LARGE_INTEGER perf_freg;
	QueryPerformanceFrequency(&perf_freg);
	m_timer_counter_freg = perf_freg.QuadPart;
	
	QueryPerformanceCounter(&m_last_time_counter);

	m_frame_start_cpu_stamp = __rdtsc();
}

void TW_App::update_timer_counter()
{
	LARGE_INTEGER current_time;
	QueryPerformanceCounter(&current_time);

	i64 counter_elapsed_time = current_time.QuadPart - m_last_time_counter.QuadPart;
	m_frame_time = f32(counter_elapsed_time) / m_timer_counter_freg;
	m_last_time_counter = current_time;

	u64 frame_end_stamp = __rdtsc();
	u64 frame_cycle_count = frame_end_stamp - m_frame_start_cpu_stamp;
	m_frame_start_cpu_stamp = frame_end_stamp;
	m_accum_cpu_cycles += frame_cycle_count;

	m_accum_frame_time += m_frame_time;
	if (m_accum_frame_time >= 1.f)
	{
		--m_accum_frame_time; 
		m_cycles_per_second = m_accum_cpu_cycles / m_frame_counter;
		m_accum_cpu_cycles = 0;
		m_frames_per_second = m_frame_counter;
		m_frame_counter = 0;
		update_title_perf_info();
	}
	
	++m_frame_counter;
}

void TW_App::update_title_perf_info()
{
	if (!m_include_perf_metrics_in_title) return;

	std::string title = m_title + " --[FPS:" + std::to_string(m_frames_per_second) + "] -- [KCPS:" + std::to_string(m_cycles_per_second / 1000) + "]";
	SetWindowTextA(m_window, title.c_str());
}

bool TW_App::load_XInput()
{
	HMODULE XInput_lib;
	if ( (XInput_lib = LoadLibraryA("xinput1_4.dll") ) || ( XInput_lib = LoadLibraryA("xinput1_3.dll") ))
	{
		XInputGetState = (x_input_get_state*)GetProcAddress(XInput_lib, "XInputGetState");
		XInputSetState = (x_input_set_state*)GetProcAddress(XInput_lib, "XInputSetState");

		return true;
	}

	return false;
}

void TW_App::update_controller_state()
{
	for (i32 i = 0; i < s_max_controllers; i++)
	{
		m_controller_state[i].m_prev = m_controller_state[i].m_curr;

		if (XInputGetState)
		{
			XINPUT_STATE xinput_state;
			if (XInputGetState(i, &xinput_state) == ERROR_SUCCESS)
			{
				auto& pad = xinput_state.Gamepad;
				m_controller_state[i].m_curr.button_states = pad.wButtons;
				
				static constexpr u32 negative_max_range = 32768;
				static constexpr u32 positive_max_range = 32767;

				if (pad.sThumbLX < 0)
					m_controller_state[i].m_curr.l_thumb_x =  (f32)pad.sThumbLX / negative_max_range;
				else
					m_controller_state[i].m_curr.l_thumb_x = (f32)pad.sThumbLX / positive_max_range;

				if (pad.sThumbLY < 0)
					m_controller_state[i].m_curr.l_thumb_y = (f32)pad.sThumbLY / negative_max_range;
				else
					m_controller_state[i].m_curr.l_thumb_y = (f32)pad.sThumbLY / positive_max_range;
				
				if (pad.sThumbRX < 0)
					m_controller_state[i].m_curr.r_thumb_x = (f32)pad.sThumbRX / negative_max_range;
				else
					m_controller_state[i].m_curr.r_thumb_x = (f32)pad.sThumbRX / positive_max_range;

				if (pad.sThumbRY < 0)
					m_controller_state[i].m_curr.r_thumb_y = (f32)pad.sThumbRY / negative_max_range;
				else
					m_controller_state[i].m_curr.r_thumb_y = (f32)pad.sThumbRY / positive_max_range;

				static constexpr u32 trigger_max_range = 255;

				m_controller_state[i].m_curr.l_trig = (f32)pad.bLeftTrigger / trigger_max_range;
				m_controller_state[i].m_curr.r_trig = (f32)pad.bRightTrigger / trigger_max_range;
			}
		}
	}
}

void TW_App::flush_events()
{
	MSG message;
	while (BOOL message_result = PeekMessage(&message, 0, 0, 0, PM_REMOVE) != 0)
	{
		if (message_result == -1) TW_TERMINATE("GetMessage encountered an error.");

		TranslateMessage(&message);
		DispatchMessageA(&message);
	}

	update_controller_state();

	std::swap(m_curr_keyboard_state, m_prev_keyboard_state);

	for (u32 i = 0; i < (u32)Key_Codes::KEY_CODE_COUNT; ++i)
		m_curr_keyboard_state[i] = (GetKeyState(s_keycode_map[i]) & (1 << 15)) > 0;
	
	update_timer_counter();
}

u32* TW_App::resize_pixel_buffer(i32 new_width, i32 new_height)
{
	TW_ASSERT(new_width >= 0, "Pixel buffer width has to be a value greater than 0");
	TW_ASSERT(new_height >= 0, "Pixel buffer hegith has to be a value greater than 0");

	m_bitmap_height = new_height;
	m_bitmap_width = new_width;

	if (m_bitmap_mem)
		delete[] m_bitmap_mem;

	m_bitmap_info.bmiHeader.biWidth = new_width;
	m_bitmap_info.bmiHeader.biHeight = new_height * -1;
	//^ windows defaults to drawring from bottom left, 
	//setting the bit map height to negative tells it to draw from top left instead.

	u64 pixel_area = (u64)new_width * new_height;
	u64 bitmap_memory_reg = pixel_area * s_bytes_per_pixel;

	m_bitmap_mem = new u32[pixel_area];
	
	return m_bitmap_mem;
}

void TW_App::update_surface()
{
	StretchDIBits(m_device_context, 0, 0, m_window_width, m_window_height, 0, 0, m_bitmap_width, m_bitmap_height, m_bitmap_mem, &m_bitmap_info, DIB_RGB_COLORS, SRCCOPY);
}

LRESULT TW_App::windows_callback(HWND win_handle, UINT message, WPARAM wparam, LPARAM lParam)
{
	LRESULT result = 0;

	switch (message)
	{
		case WM_SIZE:
		{
			RECT client_rect;
			GetClientRect(win_handle, &client_rect);

			s_app_instance->m_window_width = client_rect.right - client_rect.left;
			s_app_instance->m_window_height = client_rect.bottom - client_rect.top;
		}break;

		case WM_DESTROY:
		{
			s_app_instance->m_app_running = false;
		}break;

		case WM_CLOSE:
		{
			s_app_instance->m_app_running = false;
		}break;

		case WM_ACTIVATEAPP:
		{
			s_app_instance->m_is_focused = !s_app_instance->m_is_focused;
		}break;

		default:
		{
			result = DefWindowProcA(win_handle, message, wparam, lParam);
		}break;
	}

	return  result;
}

i64 TW_App::get_cpu_time_stamp()
{
	return __rdtsc();
}


Controller_State TW_App::get_controller_state(i32 idx)
{
	TW_ASSERT(idx >= 0 && idx < s_max_controllers, "Controller index array out of bounds!");
	return m_controller_state[idx];
}

Button_State TW_App::get_keyboard_state(Key_Codes key_code)
{
	return Button_State(m_curr_keyboard_state[(i32)key_code], m_prev_keyboard_state[(i32)key_code]);
}

bool TW_App::get_keyboard_button_down(Key_Codes key_code)
{
	return m_curr_keyboard_state[(i32)key_code];
}

void TW_App::init_bitmap_info()
{
	m_bitmap_info.bmiHeader.biSize = sizeof(m_bitmap_info.bmiHeader);
	m_bitmap_info.bmiHeader.biPlanes = 1;
	m_bitmap_info.bmiHeader.biBitCount = sizeof(i32) * 8;
	m_bitmap_info.bmiHeader.biCompression = BI_RGB;
}

