#pragma once

#define NOMINMAX
#include <Windows.h>

#include "../Types.h"

struct Sound_Output
{
	i32 samples_per_second = 48000;
	i32 bits_per_sample = 16;				// 16 bits is 2 bytes
	i32 bytes_per_sample = sizeof(i16) * 2; // == 4     !=^  ????

	i32 tone_volume = 3000;
	u32 running_idx = 0;

	inline i32 wave_period() { return samples_per_second / 256;					}
	inline i32 buffer_size() { return samples_per_second * bytes_per_sample;	}
};

struct IDirectSoundBuffer;

class TW_Sound
{
public:
	void play();
	void write();

	bool init_DSound(HWND window);

private:
	void fill_sound_buffer(DWORD byte_to_lock, DWORD bytes_to_write);

private:
	IDirectSoundBuffer* m_sound_buffer = nullptr;

	bool m_loaded_successefully = false;

	Sound_Output output;
};

