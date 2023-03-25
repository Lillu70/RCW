#include "TW_Sound.h"
#include <dsound.h>
#include <utility>


typedef HRESULT WINAPI d_sound_create(LPCGUID pcGuidDevice, LPDIRECTSOUND* ppDS, LPUNKNOWN pUnkOuter);
typedef HRESULT WINAPI d_sound_create_buffer(LPCDSBUFFERDESC pcDSBufferDesc, _Outptr_ LPDIRECTSOUNDBUFFER* ppDSBuffer, _Pre_null_ LPUNKNOWN pUnkOuter);

void TW_Sound::play()
{
	if (!m_loaded_successefully) return;

	m_sound_buffer->Play(0, 0, DSBPLAY_LOOPING);
}

void TW_Sound::write()
{
	if (!m_loaded_successefully) return;

	DWORD play_cursor;
	DWORD write_cursor;

	if (SUCCEEDED(m_sound_buffer->GetCurrentPosition(&play_cursor, &write_cursor)))
	{
		DWORD byte_to_lock = (output.running_idx * output.bytes_per_sample) % output.buffer_size();
		DWORD bytes_to_write = 0;

		if (byte_to_lock == play_cursor)
			return;
		
		else if (byte_to_lock > play_cursor)
			bytes_to_write = output.buffer_size() - byte_to_lock + play_cursor;
		
		else
			bytes_to_write = play_cursor - byte_to_lock;
		
		fill_sound_buffer(byte_to_lock, bytes_to_write);
	}
}


void TW_Sound::fill_sound_buffer(DWORD byte_to_lock, DWORD bytes_to_write)
{
	VOID* region1;
	DWORD region1_size;
	VOID* region2;
	DWORD region2_size;

	if (!SUCCEEDED(m_sound_buffer->Lock(byte_to_lock, bytes_to_write, &region1, &region1_size, &region2, &region2_size, 0))) 
		return;

	i16* sample_out = (i16*)region1;
	DWORD region1_sample_count = region1_size / output.bytes_per_sample;
	for (DWORD sample_idx = 0; sample_idx < region1_sample_count; ++sample_idx)
	{
		f32 t = (f32)TAU * (f32)output.running_idx / (f32)output.wave_period();
		f32 sine_val = sinf(t);
		i16 sample_value = (i16)(sine_val * output.tone_volume);

		*sample_out++ = sample_value;
		*sample_out++ = sample_value;
		++output.running_idx;
	}


	sample_out = (i16*)region2;
	DWORD region2_sample_count = region2_size / output.bytes_per_sample;
	for (DWORD sample_idx = 0; sample_idx < region2_sample_count; ++sample_idx)
	{
		f32 t = (f32)TAU * (f32)output.running_idx / (f32)output.wave_period();
		f32 sine_val = sinf(t);
		i16 sample_value = (i16)(sine_val * output.tone_volume);

		*sample_out++ = sample_value;
		*sample_out++ = sample_value;
		++output.running_idx;
	}

	m_sound_buffer->Unlock(region1, region1_size, region2, region2_size);
}


bool TW_Sound::init_DSound(HWND window)
{
	HMODULE Dsound_lib = LoadLibraryA("dsound.dll");

	if (!Dsound_lib)
		return false;

	d_sound_create* direct_sound_create_instance = (d_sound_create*)GetProcAddress(Dsound_lib, "DirectSoundCreate");
	if (!direct_sound_create_instance)
		return false;

	LPDIRECTSOUND dsound;
	if (!SUCCEEDED(direct_sound_create_instance(0, &dsound, 0)))
		return false;

	WAVEFORMATEX wave_format = {};
	wave_format.wFormatTag = WAVE_FORMAT_PCM;
	wave_format.nChannels = 2;
	wave_format.nSamplesPerSec = output.samples_per_second;
	wave_format.wBitsPerSample = output.bits_per_sample;
	wave_format.nBlockAlign = (wave_format.nChannels * output.bits_per_sample) / 8;
	wave_format.nAvgBytesPerSec = output.samples_per_second * wave_format.nBlockAlign;
	wave_format.cbSize = 0;

	if (!SUCCEEDED(dsound->SetCooperativeLevel(window, DSSCL_PRIORITY)))
		return false;

	DSBUFFERDESC buffer_description = {};
	buffer_description.dwFlags = DSBCAPS_PRIMARYBUFFER;
	buffer_description.dwSize = sizeof(DSBUFFERDESC);

	LPDIRECTSOUNDBUFFER primary_buffer;
	if (!SUCCEEDED(dsound->CreateSoundBuffer(&buffer_description, &primary_buffer, 0)))
		return false;
	primary_buffer->SetFormat(&wave_format);

	buffer_description.dwFlags = 0;
	buffer_description.dwBufferBytes = output.buffer_size();
	buffer_description.lpwfxFormat = &wave_format;

	if (!SUCCEEDED(dsound->CreateSoundBuffer(&buffer_description, &m_sound_buffer, 0)))
		return false;

	m_loaded_successefully = true;

	fill_sound_buffer(0, output.buffer_size());

	return m_loaded_successefully;

}
