#ifndef AUDIO_READER_H
#define AUDIO_READER_H

#include <vector>
#include <string>

#include <audiomixer/src/audioplayer.h>

#define INVALID_HANDLE ((int32_t) -1)

namespace audioreader
{

struct WavHeader;
     
size_t FileSizeLeft(int32_t handle);
int32_t OpenAudioFile(const char* fileName, int32_t handle);
void CloseAudioFile(int32_t handle);
uint32_t ReadFile(int32_t handle, char* buffer, uint32_t size);
bool ReadWavHeader(int32_t handle, WavHeader& header);

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

class AudioReader
{
public:

     /// @brief ctor
     /// @param [in] handle of the definite sound
     AudioReader(int32_t handle);

     /// @brief dtor
     ~AudioReader();

     /// @brief Open wav file
     /// @param [in] filename Name of wav file
     /// @param [in] handle of the definite sound
     /// @return true when file has opened
     bool Open(std::string filename, int32_t handle);

     /// @brief Get the next block of the sound data
     /// @param [out] buffer Pointer to a sound data buffer
     /// @param [out] size Size of the buffer
     bool NextDataBlock(uint8_t*& buffer, uint32_t& size);

     /// @brief Full sound time in seconds
     /// @return sound time in seconds
     double FullTime();

     /// @brief Size to end
     /// @return size to end in bytes
     uint32_t SizeLeft();

     ///  @brief Get Wav header
     /// @return Wav header
     const WavHeader& header();

private:
    std::vector<char> bufFromFile;
    uint32_t blockLen = 0;
    WavHeader wHeader;
    int32_t soundHandle = INVALID_HANDLE;
    double seconds = 0.;
    uint32_t bytesLeft = 0;
};

} // audioreader

#endif // AUDIO_READER_H
