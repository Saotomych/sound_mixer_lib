#ifndef AUDIO_READER_H
#define AUDIO_READER_H

#include <iostream>
#include <cstdint>
#include <vector>
#include <string>
#include <string.h>

#include "audioplayer.h"

namespace audioreader
{

struct WavHeader;
     
// На десктопе реализация через файлстримы     
// На чибике реализация через локальные структуры описания файла     

// Кросс-Платформенный интерфейс
size_t fileSizeLeft(int32_t handle);

// Кросс-Платформенный интерфейс
int32_t openAudioFile(const char* fileName, int32_t handle);

// Кросс-Платформенный интерфейс
void closeAudioFile(int32_t handle);

// Кросс-Платформенный интерфейс
uint32_t readFile(int32_t handle, char* buffer, uint32_t size);

// Кросс-Платформенный интерфейс
bool readWavHeader(int32_t handle, WavHeader& header);

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
     std::vector<char> bufFromFile;
     uint32_t blockLen;
     
     WavHeader wHeader;
     
     int32_t soundHandle;
     
     double seconds;
     uint32_t bytesLeft;
     
     std::string fname;
     
public:
     
     AudioReader(int32_t handle)
     {
          blockLen = 0;
          soundHandle = handle;
     }
     
     ~AudioReader()
     {
          audioreader::closeAudioFile( soundHandle );    
     }
     
     bool open(std::string filename, int32_t handle)
     {
          fname = filename;
          handle = audioreader::openAudioFile(filename.c_str(), handle);
          
          std::cout << "open audio file " << filename << " ... " << std::endl;
          
          if ( handle != INVALID_HANDLE )
          {
               std::cout << "audio file " << filename << " opened " << std::endl;

               bool res = audioreader::readWavHeader(handle, wHeader);
               if (res == false)
                    return false;
               
               // set vector sizes for 0,1 sec of playing
               uint32_t sz = wHeader.byteRate * 0.1;
               uint16_t bapc = wHeader.blockAlign / wHeader.numChannels;
               
               blockLen = sz / bapc * bapc;
               bytesLeft = wHeader.subchunk2Size;
               seconds =  (double)(bytesLeft) / wHeader.byteRate;

               std::cout << "size: " << bytesLeft << " bytes" << std::endl;
               
               std::cout << "timing: " << seconds << "secs" << std::endl;
               
               if ( bufFromFile.size() < blockLen )
                    bufFromFile.resize(blockLen);
               
               return true;
          }
          
          return false;
     }
     
     
     bool nextDataBlock(uint8_t*& buffer, uint32_t& size)
     {
          std::cout << "audioreader::nextDataBlock: start" << std::endl;
          
          if (blockLen)
          {
               uint32_t length = bytesLeft;
               length = (length < blockLen) ? length : blockLen;
               
               if (length)
               {
                    audioreader::readFile(soundHandle, bufFromFile.data(), length);
               }
               
               std::cout << "read data from file with len = " << length << std::endl;
               
               size = length;
               buffer = (uint8_t*) bufFromFile.data();
               
               bytesLeft -= length;
               
               return true;
          }
          
          return false;
     }
     
     /**
     *    @brief sound time in seconds
     */
     double fullTime()
     {
          return seconds;
     }
    
     /**
     *     @brief returns bytes to end
     */
     uint32_t sizeLeft()
     {
          return bytesLeft;
     }
     
     /**
      *     @brief returns Wav header
      */
     const WavHeader& header()
     {
          return wHeader;
     }
     
};

} // audioreader

#endif // AUDIO_READER_H
