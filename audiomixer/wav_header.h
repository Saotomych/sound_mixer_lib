#ifndef SOUND_MIXER_SOFTWARE_WAV_HEADER_H
#define SOUND_MIXER_SOFTWARE_WAV_HEADER_H

#include <stdint.h>

struct WavHeader
{
    uint32_t chunkId;    // Содержит символы “RIFF” в ASCII кодировке
    uint32_t chunkSize;  // оставшийся размер цепочки, начиная с этой позиции
    uint32_t format;     // символы “WAVE”
    uint32_t subchunk1Id;     // Содержит символы “fmt “
    uint32_t subchunk1Size;   // оставшийся размер подцепочки, начиная с этой позиции
    uint16_t audioFormat;     //  Для PCM = 1
    uint16_t numChannels;     //
    uint32_t sampleRate; // 44100 Гц и т.д.
    uint32_t byteRate;   // Количество байт, переданных за секунду воспроизведения.
    uint16_t blockAlign;      // Количество байт для одного сэмпла, включая все каналы
    uint16_t bitsPerSample;   // Количество бит в сэмпле
    uint32_t subchunk2Id;     // Содержит символы “data”
    uint32_t subchunk2Size;   // Количество байт в области данных.

    char in4[4];
    char in2[2];
};

#endif //SOUND_MIXER_SOFTWARE_WAV_HEADER_H
