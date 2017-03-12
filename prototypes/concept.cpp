#include <iostream>
#include <cstdint>
#include <limits.h>
#include <cstring>
#include <fstream>
#include <vector>
#include <list>
#include <cassert>
#include <algorithm>
#include <utility>
#include <memory>

#include <SDL2/SDL.h>
#include "../sdl/include/audio.h"

#include "wavheader.h"

#define NODEBUG          0
#define DBGLEVEL1   1
#define DBGLEVEL2   2
#define DBGLEVEL3   3

#define DEBUG  NODEBUG

using namespace std;

class WavInfo
{
     char bufFromFile[102400];
     uint32_t lastBlockSize;
     
     uint32_t blockLen;
     
     ifstream wavStream;
     
     WavHeader wHeader;
     
     int32_t hndl;
     
     double secondsLeft;
     uint32_t bytesLeft;
     
     string fname;
     
public:
     
     WavInfo(int32_t soundHandle)
     {
          lastBlockSize = 0;
          blockLen = 0;
          hndl = soundHandle;
     }
     
     ~WavInfo()
     {
          if (wavStream.is_open())
               wavStream.close();
     }
     
     bool open(string filename)
     {
          fname = filename;
          wavStream.open(filename, ios::in | ios::binary);
          
          if (wavStream.is_open() == true)
          {
               wHeader << wavStream;
               
               assert(!(wavStream.rdstate() & ifstream::failbit));
               assert(!(wavStream.rdstate() & ifstream::badbit));
               
               // set vector sizes for 0,1 sec of playing
               uint32_t sz = wHeader.byteRate * 0.1;
               uint16_t bapc = wHeader.blockAlign / wHeader.numChannels;

               blockLen = sz / bapc * bapc;
               bytesLeft = wHeader.subchunk2Size;
               secondsLeft =  (double)(bytesLeft) / wHeader.byteRate;

               return (wavStream.rdstate()) ? false : true;
          }
          
          return false;
     }
     
     bool dataBlock(uint8_t*& buffer, uint32_t& size)
     {
          buffer = (uint8_t*)bufFromFile;
          size = lastBlockSize;

          bytesLeft -= lastBlockSize;
          secondsLeft = (double)(bytesLeft) / wHeader.byteRate;
          return true;
     }
     
     // @brief returns as parameters: pointer to sound data buffer
     bool nextDataBlock()
     {
          if (blockLen)
          {
               int length = wHeader.chunkSize - wavStream.tellg() + 8;
               length = (length < blockLen) ? length : blockLen;
               
               if (length)
               {
                    wavStream.read(bufFromFile, length);
                    assert(!(wavStream.rdstate() & ifstream::failbit));
                    assert(!(wavStream.rdstate() & ifstream::badbit));
               }
               
               cout << "read data from file with len = " << length << endl;
               
               lastBlockSize = length;
               
               return true;
          }

          return false;
     }
     
     // @brief returns time in seconds to end
     double secLeft()
     {
          return secondsLeft;
     }
     
     // @brief returns bytes to end
     uint32_t sizeLeft()
     {
          return bytesLeft;
     }
     
     const WavHeader& header()
     {
          return wHeader;
     }
};

void fill_audio(void *udata, Uint8 *stream, int len)
{
static uint8_t* actualSound = nullptr;
static uint32_t actualLen = 0;
     /* Only play if we have data left */
     if (udata == nullptr)
     {
          return;
     }
     
     cout << "fill audio:" << ((uint64_t)actualSound) << "; " << actualLen << "; " << len << endl;
     
     WavInfo* wav = (WavInfo*) udata;
     
     if (actualLen == 0)
     {
          if ( wav->nextDataBlock() )
               wav->dataBlock(actualSound, actualLen);
          else 
               return;
     }
     
     len = ( len > actualLen ? actualLen : len );
     SDL_memcpy (stream, actualSound, len);
     actualLen -= len;
     actualSound += len;
}

inline
void openNativeAudio(WavInfo& info, SDL_AudioSpec& wanted)
{
     // platform native code begin
     /* Set the audio format */
     wanted.freq = info.header().sampleRate;
     wanted.format = AUDIO_S16;
     wanted.channels = info.header().numChannels;    /* 1 = mono, 2 = stereo */
     wanted.samples = info.header().byteRate / 10;  /* Good low-latency value for callback */
     wanted.callback = fill_audio;
     wanted.userdata = &info;
     
     /* Open the audio device, forcing the desired format */
     if ( SDL_OpenAudio(&wanted, NULL) < 0 )
     {
          cerr << "Couldn't open audio: " <<  SDL_GetError() << endl;
     }
     // platform native code end
}

int main()
{
     // Initialize SDL.
     if ( SDL_Init(SDL_INIT_AUDIO) < 0 )
          return 1;
     
     vector<string> sound = { "dtmf.wav" }; //, "cow.wav", "romans.wav" }; //, "romans.wav" };
     vector<int> startTime = { 0 }; //, 15, 50 }; //, 30 }; // with 0,1 second step
     
     WavInfo wav( 0 );
     wav.open( sound[0] );

     SDL_AudioSpec wanted;
     openNativeAudio(wav, wanted);
     SDL_PauseAudio(0);
     
     int timer = 0;
     while ( wav.sizeLeft() )
     {
          std::cout.precision(2);
          cout << "- !new step! - Timer: " << ((double)(timer)/10) << endl;
          std::cout.precision(5);
                    
          SDL_Delay(100);
          ++timer;
     }
     
     SDL_CloseAudio();
}
