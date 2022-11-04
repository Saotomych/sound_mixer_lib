/// @brief Platform dependency interface
/// @details Due to the sound API is platform dependency any sound API connected to implementation with the interface.
/// The interface must be compatible with C99 standard

#ifndef AUDIO_PLAYER_H
#define AUDIO_PLAYER_H

#include <audiomixer/wav_header.h>

#define INVALID_HANDLE (-1)
typedef uint32_t(*cbFn)(void*, uint8_t*, int);

extern "C"
{
uint32_t PlatformFileSizeLeft(int32_t handle);
int32_t PlatformOpenAudioFile(const char* fileName, int32_t handle);
void PlatformCloseAudioFile(int32_t handle);
uint32_t PlatformReadFile(int32_t handle, char* buffer, uint32_t size);
bool PlatformReadWavHeader(int32_t handle, WavHeader& header);

bool PlatformOpenDevice(const WavHeader *wavHeader, cbFn fn, void *cbUserData);
void PlatformCloseDevice();
void PlatformStartPlay();
void PlatformPausePlay();
void PlatformBreakPlay();
void PlatformDelayPlay(uint32_t ms);
}

#endif // AUDIO_PLAYER_H