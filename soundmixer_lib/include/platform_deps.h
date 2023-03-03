/// @file platform_deps.h
/// @brief Platform dependency interface
/// @details This is a set of the platform dependencies to work with
/// the sound files and hardware with the library.
/// The interface must be compatible with C99 standard.

#pragma once

#ifndef PLATFORM_DEPS_H
#define PLATFORM_DEPS_H

#include <soundmixer/wav_header.h>

#define INVALID_HANDLE (-1)

/// @brief Type for callback function calling by user for
/// the mixed sound getting
typedef uint32_t(*cbGetMixedData)(void*, uint8_t*, int);

#ifdef __cplusplus
extern "C"
{
#endif

/// @fn PlatformFileSizeLeft(int32_t handle)
/// @brief Ask rest size of file by handle.
/// @details Can be called any time while file is playing.
/// @param[in] handle handle of the sound file
/// @return rest size
uint32_t PlatformFileSizeLeft(int32_t handle);

/// @fn PlatformOpenSoundFile(const char* fileName, int32_t handle)
/// @brief Open a sound file by a file name.
/// @details Can be called during the PlaySound function. Platform has to 
/// remember handle-file association for latest requests. 
/// Handle is controlled by the library
/// @param[in] fileName file name
/// @param[in] handle file handle
/// @return handle if OK or INVALID_HANDLE if file isn't found
int32_t PlatformOpenSoundFile(const char* fileName, int32_t handle);

/// @fn PlatformCloseSoundFile(int32_t handle)
/// @brief Close a sound file by handle.
/// @details Can be called any time while file is playing.
void PlatformCloseSoundFile(int32_t handle);

/// @fn PlatformReadFile(int32_t handle, char* buffer, uint32_t size)
/// @brief Read data from a sound file
/// @details Can be called any time while file is playing. Platform has to
/// return next data block from the file
/// @param [in] handle file handle
/// @param [out] buffer data buffer
/// @param [in] size size of the data buffer
/// @return read data block size
uint32_t PlatformReadFile(int32_t handle, char* buffer, uint32_t size);

/// @fn PlatformReadWavHeader(int32_t handle, WavHeader& header)
/// @brief Read a sound file header.
/// @details Can be called during the PlaySound function. Platform must read 
/// sound header and fill the header structure
/// @param [in] handle file handle
/// @param [out] header parsed sound file header
/// @return true when header was parsed correctly
bool PlatformReadWavHeader(int32_t handle, WavHeader& header);

/// @fn PlatformOpenDevice(const WavHeader *wavHeader, 
///     cbGetMixedData fn, void *cbUserCtx)
/// @brief Initialize a user sound hardware.
/// @details Can be called every time when the PlaySound function performs for
/// the first sound file.
/// @param [in] wavHeader sound data to initialize hardware
/// @param [in] fn function to get mixed data for the next data block set
/// @param [in] cbUserCtx user context for the fn
/// @return true when the hardware is ready to work
bool PlatformOpenDevice(const WavHeader *wavHeader, 
    cbGetMixedData fn, void *cbUserCtx);

/// @fn PlatformCloseDevice()
/// @brief Deinitialize a user sound hardware.
/// @details Can be called when last sound file was finished. The library sees
/// the end of file by the PlatformFileSizeLeft function.
void PlatformCloseDevice();

/// @fn PlatformStartPlay()
/// @brief Start of playing with hardware. 
/// @details Can be called during PlaySound and ResumePlay functions
/// when PlatformOpenDevice returned true.
void PlatformStartPlay();

/// @fn PlatformPausePlay()
/// @brief Set pause for the user sound hardware.
/// @details Can be called during the Pause function.
void PlatformPausePlay();

/// @fn PlatformBreakPlay()
/// @brief Break playing for the user sound hardware.
/// @details Can be called during the BreakPlay function.
void PlatformBreakPlay();

/// @fn PlatformDelayPlay(uint32_t ms)
/// @brief Play mixed sound during some time.
/// @details Can be called during the DelayPlay function.
/// @param [in] ms playing time in ms
void PlatformDelayPlay(uint32_t ms);

#ifdef __cplusplus
}
#endif

#endif // PLATFORM_DEPS_H
