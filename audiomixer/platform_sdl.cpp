#include <iostream>
#include <fstream>
#include <cassert>

#include <SDL2/SDL.h>
#include "../sdl/include/audio.h"

#include "audioplayer.h"
#include "audioreader.h"
#include "audiomixer.h"

namespace
{
     std::ifstream files[MAX_SOUNDS];
     SDL_AudioSpec wanted;

     uint8_t* actualSound = nullptr;
     uint32_t actualLen = 0;
     
     void fill_audio(void *udata, Uint8 *stream, int len)
     {

//          std::cout << "fill audio:" << ((uint64_t)actualSound) << "; " << actualLen << "; " << len << std::endl;
          
          /* Only play if we have data left */
          if (udata == nullptr)
          {
               return;
          }
          
          audiomixer::AudioMixer* mixer = (audiomixer::AudioMixer*) udata;
          uint32_t realLen = mixer->getMixedBuffer( stream, len );

          std::cout << "fill audio: got mixed buffer with len: " << realLen << std::endl;

//          realLen = ( realLen > actualLen ? actualLen : realLen );
//
//           if ( realLen )
//           {
//                SDL_memcpy (stream, actualSound, realLen);
//           }
//           
          actualLen -= realLen;
          actualSound += realLen;
     }
}

size_t audioreader::fileSizeLeft(int32_t handle)
{
     assert(handle >= 0 && handle < MAX_SOUNDS);
     return files[handle].tellg();
}
     
int32_t audioreader::openAudioFile(const char* fileName)
{
     int32_t handle = 0;
     for ( std::ifstream& s: files )
     {
          if ( s.is_open() == false )
          {
               s.open(fileName, std::ios::in | std::ios::binary);
               if (s.is_open() == true)
               {
                    return handle;
               }
               else
               {
                    return INVALID_HANDLE;
               }
          }
          ++handle;
     }

     return INVALID_HANDLE;
}

void audioreader::closeAudioFile(int32_t handle)
{
     assert(handle >= 0 && handle < MAX_SOUNDS);
     
     if (files[handle].is_open())
          files[handle].close();
}
    
uint32_t audioreader::readFile(int32_t handle, char* buffer, uint32_t size)
{
     assert(handle >= 0 && handle < MAX_SOUNDS);
     assert(buffer != nullptr);
     assert(size != 0);

     std::cout << "audioreader::readFile: size = " << size << std::endl;
     
     std::ifstream& s = files[handle];
     s.read(buffer, size);

     assert(!(s.rdstate() & std::ifstream::failbit));
     assert(!(s.rdstate() & std::ifstream::badbit));
     
     return size;
}

bool audioreader::readWavHeader(int32_t handle, WavHeader& header)
{
     assert(handle >= 0 && handle < MAX_SOUNDS);
     std::ifstream& s = files[handle];
     
     s.read((char*)&header, 36);
     s.ignore(header.subchunk1Size - 16);
     s.read((char*) &(header.subchunk2Id), 8);
     
     assert(!(s.rdstate() & std::ifstream::failbit));
     assert(!(s.rdstate() & std::ifstream::badbit));

     if ( 0x61746164 != header.subchunk2Id )   // not data
     {
          s.ignore(header.subchunk2Size);
          s.read((char*) &(header.subchunk2Id), 8);

          assert(!(s.rdstate() & std::ifstream::failbit));
          assert(!(s.rdstate() & std::ifstream::badbit));
     }
     
     if ( 0x61746164 != header.subchunk2Id )   // not data
     {
          memset((char*) &header, 0, sizeof(WavHeader));
     }

     if (s.rdstate() & std::ifstream::failbit)
          return false;
     
     if (s.rdstate() & std::ifstream::badbit)
          return false;
     
     return true;
}

bool audioplayer::openDevice(audioreader::AudioReader* reader, audiomixer::AudioMixer* mixer)
{
     std::cout << "SDL audio open..." << std::endl;
     
     if (SDL_Init(SDL_INIT_AUDIO) < 0)
     {
        std::cout << "Critical: No SDL." << std::endl; 
        return false;
     }
     
     /* Set the audio format */
     wanted.freq = reader->header().sampleRate;
     wanted.format = AUDIO_S16;
     wanted.channels = reader->header().numChannels;    /* 1 = mono, 2 = stereo */
     wanted.samples = reader->header().byteRate / 10;  /* Good low-latency value for callback */
     wanted.callback = fill_audio;
     wanted.userdata = mixer;
     
     /* Open the audio device, forcing the desired format */
     if ( SDL_OpenAudio(&wanted, NULL) < 0 )
     {
          std::cout << "Critical: Open SDL audio failed." << std::endl; 
          return false;
     }
     
     std::cout << "SDL audio opened" << std::endl;
     
     return true;
}

void audioplayer::closeDevice()
{
     std::cout << "close SDL device" << std::endl;
     SDL_CloseAudio();
}

void audioplayer::startPlay()
{
     std::cout << "resume SDL audio" << std::endl;
     SDL_PauseAudio(0);
}

void audioplayer::pausePlay()
{
     std::cout << "pause SDL audio" << std::endl;
     SDL_PauseAudio(1);
}

void audioplayer::breakPlay()
{
     std::cout << "abort SDL audio" << std::endl;
     SDL_PauseAudio(1);
     SDL_CloseAudio();
}

void audioplayer::delayPlay( uint32_t ms)
{
     SDL_Delay( ms );
}
