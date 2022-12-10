#ifndef SOUND_MIXER_SOFTWARE_WAV_HEADER_H
#define SOUND_MIXER_SOFTWARE_WAV_HEADER_H

#include <stdint.h>

struct WavHeader
{
    uint32_t chunkId;       // Contains “RIFF” in ASCII
    uint32_t chunkSize;     // Rest of sound chain from the actual position
    uint32_t format;        // Contains “WAVE” in ASCII
    uint32_t subchunk1Id;   // Contains “fmt “ in ASCII
    uint32_t subchunk1Size; // Rest of sound subchain from the actual position
    uint16_t audioFormat;   // For PCM = 1
    uint16_t numChannels;   //
    uint32_t sampleRate;    // 11025 Hz
    uint32_t byteRate;      // Bytes per second of the playing
    uint16_t blockAlign;    // Bytes per sample for all channels
    uint16_t bitsPerSample; // Bits per sample
    uint32_t subchunk2Id;   // Contains “data”  in ASCII
    uint32_t subchunk2Size; // Sound data size

    char in4[4];
    char in2[2];
};

#endif //SOUND_MIXER_SOFTWARE_WAV_HEADER_H
