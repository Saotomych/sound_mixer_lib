/// @brief Platform dependency interface
/// @details Due to the sound API is platform dependency any sound API connected to implementation with the interface.
/// The interface must be compatible with C99 standard

#ifndef AUDIO_PLAYER_H
#define AUDIO_PLAYER_H

#include <audiomixer/wav_header.h>

#define INVALID_HANDLE (-1)
typedef uint32_t(*cbFn)(void*, uint8_t*, int);

bool OpenDevice(const WavHeader* wavHeader, cbFn fn, void* cbUserData);
void CloseDevice();
void StartPlay();
void PausePlay();
void BreakPlay();
void DelayPlay(uint32_t ms);

#endif // AUDIO_PLAYER_H