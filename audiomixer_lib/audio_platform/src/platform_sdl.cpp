#include <fstream>
#include <cassert>
#include <unordered_map>

#include <SDL2/SDL.h>
#include <sdl/include/audio.h>

#include <audio_platform/audio_platform.h>

namespace {
    std::unordered_map <uint32_t, std::ifstream> files;
    SDL_AudioSpec wanted;
    cbFn userFn = nullptr;

    uint8_t *actualSound = nullptr;
    uint32_t actualLen = 0;

    void FillAudio(void *udata, Uint8 *stream, int len) {
        if (udata == nullptr) {
            return;
        }

        uint32_t realLen = userFn(udata, stream, len);
        actualLen -= realLen;
        actualSound += realLen;
    }

}

extern "C"
uint32_t PlatformFileSizeLeft(int32_t handle)
{
     return files[handle].tellg();
}

extern "C"
int32_t PlatformOpenAudioFile(const char* fileName, int32_t handle)
{
     if (files.find(handle) == files.end())
         files.emplace(handle, std::ifstream());

     std::ifstream& s = files[handle];

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

     return INVALID_HANDLE;
}

extern "C"
void PlatformCloseAudioFile(int32_t handle)
{
     if (files[handle].is_open())
          files[handle].close();
     files.erase(handle);
}

extern "C"
uint32_t PlatformReadFile(int32_t handle, char* buffer, uint32_t size)
{
     assert(buffer != nullptr);
     assert(size != 0);

     std::ifstream& s = files[handle];
     s.read(buffer, size);

     assert(!(s.rdstate() & std::ifstream::failbit));
     assert(!(s.rdstate() & std::ifstream::badbit));
     
     return size;
}

extern "C"
bool PlatformReadWavHeader(int32_t handle, WavHeader& header)
{
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

extern "C"
bool PlatformOpenDevice(const WavHeader* wavHeader, cbFn fn, void* cbUserData)
{
     if (SDL_Init(SDL_INIT_AUDIO) < 0)
     {
        return false;
     }
     
     /* Set the audio format */
     wanted.freq = wavHeader->sampleRate;
     wanted.format = AUDIO_S16;
     wanted.channels = wavHeader->numChannels;    /* 1 = mono, 2 = stereo */
     wanted.samples = wavHeader->sampleRate / 10;  /* Good low-latency value for callback */
     wanted.callback = FillAudio;
     wanted.userdata = cbUserData;
     userFn = fn;
     
     /* Open the audio device, forcing the desired format */
     if ( SDL_OpenAudio(&wanted, NULL) < 0 )
     {
          return false;
     }

//     std::cout << "SDL audio has opened" << std::endl;

     return true;
}

extern "C"
void PlatformCloseDevice()
{
//     std::cout << "close SDL device" << std::endl;
     SDL_CloseAudio();
}

extern "C"
void PlatformStartPlay()
{
//     std::cout << "resume SDL audio" << std::endl;
     SDL_PauseAudio(0);
}

extern "C"
void PlatformPausePlay()
{
//     std::cout << "pause SDL audio" << std::endl;
     SDL_PauseAudio(1);
}

extern "C"
void PlatformBreakPlay()
{
//     std::cout << "abort SDL audio" << std::endl;
     SDL_PauseAudio(1);
     SDL_CloseAudio();
}

extern "C"
void PlatformDelayPlay( uint32_t ms)
{
//     std::cout << "delay play SDL audio" << std::endl;
     SDL_Delay( ms );
}
