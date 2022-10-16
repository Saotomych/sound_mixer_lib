#ifndef AUDIO_MIXER_H
#define AUDIO_MIXER_H

#include <list>
#include <algorithm>
#include <utility>
#include <memory>
#include <cassert>
#include <mutex>
#include <memory>

#include <audiomixer/src/audioreader.h>

namespace audiomixer
{

class AudioMixer
{
public:
     
     AudioMixer(): nextHandle(0), fullSecondLeft(0.), fullSizeLeft(0) {}
     
     uint32_t GetMixedBuffer(uint8_t* mixedData, uint32_t length)
     {
          assert(mixedData != nullptr);

          std::unique_lock<std::mutex> lock( mtSoundReaders );
          uint8_t* audioBuffer = nullptr;
          uint32_t audioSize = 0;
          uint32_t maxSize = audioSize;

          std::fill( mixedData, &mixedData[length], 0 );
          for (auto& reader: readers )
          {
               if ( reader.NextDataBlock(audioBuffer, audioSize) == false || audioSize == 0)
               {
                    deleteSound( idx );
                    continue;
               }

               assert( audioSize <= length );
               assert( audioBuffer != nullptr );

               if ( maxSize < audioSize )
                    maxSize = audioSize;
               
               uint8_t* dt = audioBuffer;
               uint8_t* mx = mixedData;
               for ( uint32_t j = 0; j < audioSize; j = j + bapc )
               {
                    int32_t base = (int32_t) TakeValue(mx);
                    int32_t adder = (int32_t) TakeValue(dt);
                    int32_t val = (base + adder) * vl / ( vl + abs(base * adder) / vl );
                    
                    PutValue(mx, val);
                    
                    dt += bapc;
                    mx += bapc;
               }
          }
          
          fullSecondLeft -= (double) maxSize / btrate;
          if ( abs(fullSecondLeft) < 0.01 )
               fullSecondLeft = 0.;
          
          fullSizeLeft -= maxSize;
          
          return maxSize;
     }
     
     int32_t AddSound( std::string fileName )
     {
          std::unique_lock<std::mutex> lock( mtSoundReaders );

          auto ar = readers.emplace_back(nextHandle);
          if ( ar->open( fileName, nextHandle ) )
          {
               const audioreader::WavHeader& wh = ar.header();
               if ( fullSecondLeft == 0. )
               {
                    if ( audioplayer::openDevice( &ar, this ) ) {
                        audioplayer::startPlay();
                    } else {
                        readers.erase(ar);
                        return INVALID_HANDLE;
                    }
               }
               
               if ( fullSizeLeft < ar->sizeLeft() )
               {
                    fullSecondLeft = ar->fullTime();
                    fullSizeLeft = ar->sizeLeft();
               }

               ba = wh.blockAlign;
               bps = wh.bitsPerSample;
               bapc = ba;          // assumption: stereo has only one channel here
               vl = (1 << (bps-1)) - 1;
               btrate = wh.byteRate;
               return nextHandle;
          }

          readers.erase(ar);
          return INVALID_HANDLE;
     }
     
     void RemoveSound( int32_t handle )
     {
          assert(handle >= 0 && handle < MAX_SOUNDS);

          std::unique_lock<std::mutex> lock( mtSoundReaders );
          DeleteSound( handle );
     }

     double SecLeft()
     {
          std::unique_lock<std::mutex> lock( mtSoundReaders );
          return fullSecondLeft;
     }
     
     uint32_t SizeLeft()
     {
          std::unique_lock<std::mutex> lock( mtSoundReaders );
          return fullSizeLeft;
     }

    std::list<audioreader::AudioReader> readers;
    std::mutex mtSoundReaders;

    uint16_t ba;   // block align
    uint16_t bapc; // bit per channel
    uint16_t bps;  // bit per sample
    double btrate; // timing per second
    int32_t vl;
    int32_t nextHandle;
    double fullSecondLeft;
    uint32_t fullSizeLeft;

    int32_t TakeValue(uint8_t* buf)
    {
        int32_t v = 0;
        uint32_t shift = 0;
        for(uint8_t i = 0; i < bapc; ++i)
        {
            uint32_t t = (uint32_t)(*buf) << shift;
            v |= t;
            shift += 8;
            ++buf;
        }

        uint8_t sign = ( v & (1 << (shift - 1)) ) ? 0xFF : 0;
        for(uint8_t i = bapc; i < sizeof(v); ++i)
        {
            v |= (uint32_t)(sign) << shift;
            shift += 8;
        }
        return v;
    }

    void PutValue(uint8_t* buf, int32_t value)
    {
        uint8_t* pVal = (uint8_t*) (&value);
        for (uint8_t i = 0; i < bapc; ++i)
        {
            *buf = *pVal;
            ++buf;
            ++pVal;
        }

        if ( value < 0 )
        {
            *(buf-1) |= 0x80;
        }
    }

    void DeleteSound(uint32_t idx)
    {
        assert(idx >= 0 && idx < MAX_SOUNDS);
        readers[idx].reset( nullptr );
        if ( nextHandle == INVALID_HANDLE )
        {
            nextHandle = idx;
        }
    }

};

} // audiomixer

#endif // AUDIO_MIXER_H
