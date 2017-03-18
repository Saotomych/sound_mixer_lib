#ifndef AUDIO_MIXER_H
#define AUDIO_MIXER_H

#include <iostream>
#include <list>
#include <algorithm>
#include <utility>
#include <memory>
#include <cassert>
#include <mutex>
#include <memory>

#include "audioreader.h"

namespace audiomixer
{

class AudioMixer
{
     std::unique_ptr<audioreader::AudioReader> readers[MAX_SOUNDS];
     std::mutex mtSoundReaders;
     
     uint16_t ba;   // block align 
     uint16_t bapc; // bit per channel
     uint16_t bps;  // bit per sample
     
     double btrate; // timing per second
     
     int32_t vl;
     
     int32_t nextHandle;
     
     double fullSecondLeft;
     uint32_t fullSizeLeft;
     
     inline
     int32_t takeValue(uint8_t* buf)
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
          
          if (DEBUG == DBGLEVEL3)
               std::cout << "take value: " << v << std::endl;
          
          return v;
     }
     
     inline
     void putValue(uint8_t* buf, int32_t value)
     {
          if (DEBUG == DBGLEVEL3)
               std::cout << "put value: " << value << std::endl;
          
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
     
     void deleteSound(uint32_t idx)
     {
          assert(idx >= 0 && idx < MAX_SOUNDS);
          
          std::cout << "Sound " << idx << " deleted." << std::endl;
          
          readers[idx].reset( nullptr );
          
          if ( nextHandle == INVALID_HANDLE )
          {
               nextHandle = idx;
          }
     }
     
public:
     
     AudioMixer(): nextHandle(0), fullSecondLeft(0.), fullSizeLeft(0) {}
     
     // mixedData - in / out
     // length - in
     uint32_t getMixedBuffer(uint8_t* mixedData, uint32_t length)
     {
          std::unique_lock<std::mutex> lock( mtSoundReaders );
          
          std::cout  << "start new mixed buffer with len " << length << std::endl; 
          
          assert(mixedData != nullptr);
          
          uint8_t* audioBuffer = nullptr;
          uint32_t audioSize = 0;
          uint32_t maxSize = audioSize;

          std::fill( mixedData, &mixedData[length], 0 );

          for ( uint32_t idx = 0; idx < MAX_SOUNDS; ++idx )
          {
               if ( readers[idx].get() == nullptr )
               {
                    continue;
               }
               
               if ( readers[idx]->nextDataBlock(audioBuffer, audioSize) == false || audioSize == 0)
               {
                    deleteSound( idx );
                    continue;
               }

               assert( audioSize <= length );
               assert( audioBuffer != nullptr );
               
               std::cout << "begin mix from reader: " << idx << std::endl; 
               
               if ( maxSize < audioSize )
                    maxSize = audioSize;
               
               uint8_t* dt = audioBuffer;
               uint8_t* mx = mixedData;
               for ( uint32_t j = 0; j < audioSize; j = j + bapc )
               {
                    int32_t base = (int32_t) takeValue(mx);
                    int32_t adder = (int32_t) takeValue(dt);
                    int32_t val = (base + adder) * vl / ( vl + abs(base * adder) / vl );
                    
                    putValue(mx, val);
                    
                    dt += bapc;
                    mx += bapc;
               }
               
               std::cout << "end mix from reader: " << idx << std::endl; 
          }
          
          fullSecondLeft -= (double) maxSize / btrate;
          if ( abs(fullSecondLeft) < 0.01 )
               fullSecondLeft = 0.;
          
          fullSizeLeft -= maxSize;
          
          return maxSize;
     }
     
     int32_t addSound( std::string fileName )
     {
          std::unique_lock<std::mutex> lock( mtSoundReaders );

          nextHandle = INVALID_HANDLE;
          for ( int32_t i = 0; i < MAX_SOUNDS; ++i )
          {
               if ( readers[i].get() == nullptr )
               {
                    nextHandle = i;
                    break;
               }
          }

          if (nextHandle == INVALID_HANDLE)
          {
               std::cout << "Critical: Free handle not found" << std::endl;
               return INVALID_HANDLE;
          }
          
          readers[nextHandle].reset( new audioreader::AudioReader(nextHandle) );
          audioreader::AudioReader& ar = *(readers[nextHandle]);
          if ( ar.open( fileName, nextHandle ) == true )
          {
               
               const audioreader::WavHeader& wh = ar.header();

               if ( fullSecondLeft == 0. )
               {
                    if ( audioplayer::openDevice( &ar, this ) == true )
                    {
                         audioplayer::startPlay();
                    }
               }
               
               if ( fullSizeLeft < ar.sizeLeft() )
               {
                    fullSecondLeft = ar.secLeft();
                    fullSizeLeft = ar.sizeLeft();
               }

               ba = wh.blockAlign;
               bps = wh.bitsPerSample;
               bapc = ba;          // assumption: stereo has only one channel here
               vl = (1 << (bps-1)) - 1;
               btrate = wh.byteRate;

               std::cout << "file attached to handle reader: " << nextHandle << std::endl;
               
               return nextHandle;
          }

          readers[nextHandle].reset(nullptr);
          
          return INVALID_HANDLE;
     }
     
     void removeSound( int32_t handle )
     {
          assert(handle >= 0 && handle < MAX_SOUNDS);

          std::unique_lock<std::mutex> lock( mtSoundReaders );

          deleteSound( handle );
     }

     double secLeft()
     {
          return fullSecondLeft;
     }
     
     uint32_t sizeLeft()
     {
          return fullSizeLeft;
     }
     
};

} // audiomixer

#endif // AUDIO_MIXER_H
