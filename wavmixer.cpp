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
#include "sdl/include/audio.h"

#include "wavheader.h"

#define NODEBUG          0
#define DBGLEVEL1   1
#define DBGLEVEL2   2
#define DBGLEVEL3   3

#define DEBUG  NODEBUG

using namespace std;

// Смешать звук это непростая задача
// Теория относительности даёт ответ на этот вопрос. 
// Она расширяет понятие принципа относительности, распространяя его и на оптические процессы. 
// Правило сложение скоростей при этом не отменяется совсем, а лишь уточняется для больших скоростей 
// с помощью преобразования Лоренца:
// vrel = (v1+v2)/(1+(v1*v2)/c^2) 
// Можно заметить, что в случае, когда , преобразования Лоренца переходят в преобразования Галилея. 
// Это говорит о том, что специальная теория относительности совпадает с механикой Ньютона при скоростях, 
// малых по сравнению со скоростью света. 
// Это объясняет, каким образом сочетаются эти две теории — первая является уточнением второй.

// меняем скорость света на максимальное значение семпла,
// а складываемые скорости -  на текушие значения семплов.

/*
 *        for (int i = 0; i < bufflen; ++i) {
 *            int max_val_square = 1073741824; 
 *         // нaша скорость света -> max short ^ 2 => 32768*32768  (ибо я работаю с 16bit-ным(знаковым) звуком)
 *            float a = buf1[i];
 *            float b = buf2[i];
 *            float rel_samp = (a + b) / (1 + (a * b) / max_val_square);
 *            out_buf[i] = rel_samp;
 *        }
 */        

// Назначение класса
// Хранить заголовок и вычитывать следующую порцию данных упрежденно
class WavInfo
{
     vector<char> buf1;
     vector<char> buf2;
     
     char* bufFromFile;
     char* bufToMixer;
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
     
     WavInfo(const WavInfo& rhs)
     {
          assert(wavStream.is_open() == false);
          hndl = rhs.hndl;
     }
     
     WavInfo operator=(const WavInfo& rhs)
     {
          assert(false);
          hndl = rhs.hndl;
          return *this;
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
               buf1.resize(sz);
               buf2.resize(sz);
               bufFromFile = buf1.data();
               bufToMixer = buf2.data();
               
               bytesLeft = wHeader.subchunk2Size;
               secondsLeft =  (double)(bytesLeft) / wHeader.byteRate;
               
               nextDataBlock();
               
               return (wavStream.rdstate()) ? false : true;
          }
          
          return false;
     }
     
     void clear() 
     {
          if (wavStream.is_open())
               wavStream.close();
     }

     bool dataBlock(uint8_t*& buffer, uint32_t& size)
     {
          buffer = (uint8_t*)bufToMixer;
          size = lastBlockSize;
          return true;
     }
     
     // @brief returns as parameters: pointer to sound data buffer
     bool nextDataBlock()
     {
          if (blockLen)
          {
               int length = bytesLeft; //wHeader.chunkSize - wavStream.tellg() + 8;
               length = (length < blockLen) ? length : blockLen;
               
               if (length)
               {
                    wavStream.read(bufFromFile, length);
                    assert(!(wavStream.rdstate() & ifstream::failbit));
                    assert(!(wavStream.rdstate() & ifstream::badbit));
               }
               
               cout << "read data from file with len = " << length << endl;
               
               lastBlockSize = length;
               bytesLeft -= lastBlockSize;
               secondsLeft = (double)(bytesLeft) / wHeader.byteRate;
               
               swap(bufFromFile, bufToMixer);
               
//               cout << "got next data block for handle: " << hndl << " (" << fname << ")" << endl;
          }
          
          return (lastBlockSize == 0) ? false : true;
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
     
     int32_t handle()
     {
          return hndl;
     }
     
     const WavHeader& header()
     {
          return wHeader;
     }
     
     const string& name()
     {
          return fname;
     }
     
};

class MixedSound
{
     uint8_t mixedData[1024 * 64];
     uint8_t mixedSize[1024 * 64];
     uint16_t ba;
     uint16_t bapc;
     uint16_t bps;
     int32_t vl;
     
     uint32_t index;
     
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
               cout << "take value: " << v << endl;
          
          return v;
     }
     
     inline
     void putValue(uint8_t* buf, int32_t value)
     {
          if (DEBUG == DBGLEVEL3)
               cout << "put value: " << value << endl;
          
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
     
public:
     
     MixedSound(uint16_t blockAlign, uint16_t bitsPerSample): 
     ba(blockAlign), bps(bitsPerSample), index(0) 
     {
          bapc = ba;          // stereo has only one channel here
          vl = (1 << (bps-1)) - 1;
     }
     
     uint8_t* mix(vector<uint8_t*>& data, vector<uint32_t>& size)
     {
          uint32_t maxsize = 0;
          for (auto v: size)
          {
               if (v > maxsize )
                    maxsize = v;
          }
          
          return mix( data, size, maxsize );
     }
     
     uint8_t* mix(vector<uint8_t*>& data, vector<uint32_t>& size, uint32_t maxsize)
     {
          copy(&(data[0])[0], &(data[0])[size[0]], &mixedData[0]);
          fill(&mixedData[size[0]], &mixedData[maxsize], 0);
          fill(&mixedSize[0], &mixedSize[size[0]], 1);
          fill(&mixedSize[size[0]], &mixedSize[maxsize], 0);
          
          for ( uint32_t i = 1; i < data.size(); ++i )
          {
               uint32_t sz = size[i] / bapc;
               uint8_t* dt = data[i];
               uint8_t* mx = mixedData;
               for ( uint32_t j = 0; j < sz; ++j )
               {
                    int32_t base = (int32_t) takeValue(mx);
                    int32_t adder = (int32_t) takeValue(dt);
                    int32_t val = (base + adder) * vl / ( vl + abs(base * adder) / vl );
                    
                    putValue(mx, val);
                    
                    ++mixedSize[j];
                    dt += bapc;
                    mx += bapc;
               }
          }
      
          return mixedData;
     }
};

class SndMixer
{
     
     list<WavInfo> sounds;
     int32_t soundHandle;
     WavInfo empty;
     
     unique_ptr<MixedSound> mixedSound;

     SDL_AudioSpec wanted;

     static int32_t actualLen;
     
     static uint8_t* actualSound;
     
     struct Timing
     {
          Timing(): secPlayed(0.0), secLeft(0.0) {}
          double secPlayed;
          double secLeft;
     } t;
     
     WavInfo& wav(int32_t handle)
     {
          auto it = std::find_if(sounds.begin(), sounds.end(), [handle](WavInfo& w)->bool { return w.handle() == handle; } );  
          if (it == sounds.end())
               return empty;
          
          return *it;
     }
     
     inline
     void generateWavHeader(int32_t handle)
     {
          WavInfo& info = wav(handle);
          generateWavHeader(info);
     }
     
     inline
     void generateWavHeader(WavInfo& info)
     {
          t.secLeft = info.secLeft();
          cout << "!!! Generate new header with etalon header " << info.name() << " and time lenght = " << t.secLeft << endl;
     }
     
     // Platform native code
     inline
     void openNativeAudio(int32_t handle)
     {
          WavInfo& info = wav(handle);
          openNativeAudio(info, handle);          
     }

     inline
     void openNativeAudio(WavInfo& info, int32_t handle)
     {
          // platform native code begin
          /* Set the audio format */
          wanted.freq = info.header().sampleRate;
          wanted.format = AUDIO_S16;
          wanted.channels = info.header().numChannels;    /* 1 = mono, 2 = stereo */
          wanted.samples = info.header().byteRate / 10;  /* Good low-latency value for callback */
          wanted.callback = fill_audio;
          wanted.userdata = this;
          
          /* Open the audio device, forcing the desired format */
          if ( SDL_OpenAudio(&wanted, NULL) < 0 )
          {
               cerr << "Couldn't open audio: " <<  SDL_GetError() << endl;
          }
          // platform native code end
     }
     
     static 
     void fill_audio(void *udata, Uint8 *stream, int len)
     {
          /* Only play if we have data left */
          if (actualSound == nullptr)
          {
               if ( ((SndMixer*) udata)->playingSound() == false )
               {
                    cout << "fillaudio stop" << endl;
                    actualSound = nullptr;
                    actualLen = 0;
                    return;
               }
          }

          /* Mix as much data as possible */
          len = ( len > actualLen ? actualLen : len );
          SDL_memcpy (stream, actualSound, len);                        // simply copy from one buffer into the other
          actualSound = nullptr;
          actualLen = 0;
               
          if ( ((SndMixer*) udata)->playingSound() == false )
          {
               actualSound = nullptr;
               actualLen = 0;
          }
     }
     // Platform native code end
     
public:
     
     SndMixer(): empty(-1), soundHandle(-1), t() {}
     
     inline
     void startNativeAudio()
     {
          SDL_PauseAudio(0);
     }
     
     inline
     void closeNativeAudio()
     {
          SDL_Delay(10);         /* Sleep 1/100 second */
          SDL_CloseAudio();
     }
     
     bool playingSound()
     {
          
          // Sound Refresh
          for (auto it = sounds.begin(); it != sounds.end(); it)
          {
               if (it->sizeLeft() == 0)
               {
                    auto eraseit = it;
                    ++it;
                    sounds.erase(eraseit);
               }
               else
               {
                    ++it;
               }
          }
          
          cout << "Timing: played: " << t.secPlayed << "; left: " << t.secLeft << endl;
          
          if (sounds.empty())
          {
               cout << "Sound array is empty" << endl;
               
               return false;
          }
          
//          when global time left equals 0 new wave header must generate
          if (t.secLeft <= 0.1)
          {
               double maxLen = 0;
               uint32_t hndl = 0;
               for(WavInfo& v: sounds)
               {
                    if (v.secLeft() > maxLen)
                    {
                         maxLen = v.secLeft();
                         hndl = v.handle();
                    }
               }
               
               if ( maxLen > (t.secLeft + (0.1 / 8)))
               {
                    generateWavHeader(hndl);
//                    openNativeAudio(hndl);
               }
          }
          
          // Global time counters refresh
          t.secPlayed += 0.1;

          t.secLeft = 0;
          
          // Preparing of sound buffers
          vector<uint8_t*> data;
          vector<uint32_t> size;
          uint32_t maxsize = 0;
          
          bool nextRdy = false;
          for(WavInfo& v: sounds)
          {
               uint8_t* d;
               uint32_t s;
               nextRdy = v.nextDataBlock();
               v.dataBlock(d, s);
               if (s)
               {
                    data.push_back(d);
                    size.push_back(s);
               }
               
               if ( s > maxsize )
                    maxsize = s;
               
               // Time detector
               if (v.secLeft() > t.secLeft)
                    t.secLeft = v.secLeft();
               
               cout << v.name() << " is left " << v.secLeft() << " seconds and " << v.sizeLeft() << " bytes." << endl;
               //     cout << "Playing size = " << s << endl;
          }

          if (nextRdy)
          {
               // TODO Mix sound buffers
               assert( mixedSound.get() );
               actualSound = mixedSound->mix(data, size, maxsize);
               actualLen = maxsize;

               // TODO: play mixed sound with DMA
               // Start
     //          startNativeAudio();
               
               // Wait of hardware
     //           while ( actualSound )
     //           {
     //                SDL_Delay(50);         /* Sleep 1/100 second */
     //           }
          }
          
          return nextRdy;
     }
     
     uint32_t PlaySound(string filename)
     {
          ++soundHandle;
          
          sounds.push_back( WavInfo(soundHandle));
          
          WavInfo& info = wav(soundHandle);
          
          if (info.open(filename) == false)
               return -1;
          
          if (t.secLeft == 0.0 && t.secPlayed == 0.0)
          {
               t.secLeft = info.secLeft();
               mixedSound.reset( new MixedSound(info.header().blockAlign, info.header().bitsPerSample) );
               generateWavHeader(info);
               openNativeAudio(info, soundHandle);
               
//                actualLen = info.header().byteRate / 10;
//                actualSound = nullptr;
//                startNativeAudio();
//                // Wait of hardware
//                SDL_Delay(50);       
          }
          
          // prototype of cycle reading wav
          cout << "Play begin: " << endl << filename << " is left " << info.secLeft() << " seconds and " << info.sizeLeft() << " bytes." << endl;
          
          return soundHandle;
     }
     
     void StopSound(uint32_t handle)
     {
          
     }
     
     void PauseSound(uint32_t handle)
     {
          
     }
     
     void ResumeSound(uint32_t handle)
     {
          
     }
     
     void Pause()
     {
          
     }
     
     void Resume()
     {
          
     }
     
     Timing timing()
     {
          return t;
     }
     
};

int32_t SndMixer::actualLen = 0;
uint8_t* SndMixer::actualSound = nullptr;

void clipMixedTest1byte()
{
     vector<int8_t> data1 = { -30, 30, 20, 30, 40, 20 };
     vector<int8_t> data2 = { -30, 30, 40 };
     vector<int8_t> data3 = { -30, 30, 30, 40, 30, 30 };
     vector<int8_t> data4 = { -30, 30,  0, 20, 20, 40 };
     vector<int8_t> expected = 
     { -94, 94, 78, 78, 78, 78 };
     
     vector<uint8_t*> data = { (uint8_t*)data1.data(), (uint8_t*)data2.data(), (uint8_t*)data3.data(), (uint8_t*)data4.data() };
     vector<uint32_t> size = { 6, 3, 6, 6 };
     
     MixedSound mxsnd(1, 8);
     int8_t* mxData = (int8_t*) mxsnd.mix(data, size, 6);
     
     uint32_t i = 0;
     for (int8_t v: expected)
     {
          cout << +v << " - " << +mxData[i] << endl;
          assert( v == mxData[i]);
          ++i;
     }
}

void clipMixedTest3byte1()
{
     vector<uint8_t> data1 = { 0xFF,  0x7F };
     vector<uint8_t> data2 = { 0xFF,  0x7F };
     vector<uint8_t> data3 = { 0x03, 0x80 };
     vector<uint8_t> data4 = { 0xFF,  0x7F };
     vector<uint8_t> expected = 
     { 0xFF, 0x7F };
     
     vector<uint8_t*> data = { data1.data(), data2.data(), data3.data(), data4.data() };
     vector<uint32_t> size = { 2, 2, 2, 2 };
     
     MixedSound mxsnd(2, 16);
     uint8_t* mxData = mxsnd.mix(data, size, 3);
     
     uint32_t i = 0;
     for (uint8_t v: expected)
     {
          cout << hex << +v << " - " << +mxData[i] << endl;
          assert( v == mxData[i]);
          ++i;
     }
     cout << dec;
}

void clipMixedTest3byte2()
{
     vector<uint8_t> data1 = { 0xFF,  0x7F };
     vector<uint8_t> data3 = { 0xFF,  0x7F };
     vector<uint8_t> data2 = { 0x03, 0x80 };
     vector<uint8_t> data4 = { 0xFF,  0x7F };
     vector<uint8_t> expected = 
     { 0xFF, 0x7F };
     
     vector<uint8_t*> data = { data1.data(), data2.data(), data3.data(), data4.data() };
     vector<uint32_t> size = { 2, 2, 2, 2 };
     
     MixedSound mxsnd(2, 16);
     uint8_t* mxData = mxsnd.mix(data, size, 3);
     
     uint32_t i = 0;
     for (uint8_t v: expected)
     {
          cout << hex << +v << " - " << +mxData[i] << endl;
          assert( v == mxData[i]);
          ++i;
     }
     cout << dec;
}

void mixAlgorithmTest()
{
     // Mixed sound tests
     cout << ">>> new test " << endl;
     clipMixedTest1byte();
     cout << ">>> new test " << endl;
     clipMixedTest3byte1();
     cout << ">>> new test " << endl;
     clipMixedTest3byte2();  
}

int main()
{
     // Initialize SDL.
     if (SDL_Init(SDL_INIT_AUDIO) < 0)
          return 1;
     
     vector<string> sound = { "road16bit11025.wav", "cow.wav", "romans.wav", "falling16bit.wav", "falling16bit.wav",
          "falling16bit.wav", "cow.wav", "romans.wav", "cow.wav"
     };

//      vector<string> sound = { "highlands16bit.wav", "silence.wav", "silence.wav", "silence.wav", "silence.wav",
//           "silence.wav", "silence.wav", "silence.wav", "silence.wav"
//      };

     vector<int> startTime = { 0, 30, 60, 90, 120, 150, 180, 210, 240 }; // with 0,1 second step
     
     SndMixer mixer;
     
     int timer = 0;
     do{
          std::cout.precision(2);
          cout << "- !new step! - Timer: " << ((double)(timer)/10) << endl;
          std::cout.precision(5);
          
          for (int i=0; i < sound.size(); ++i) 
          {
               if (timer == startTime[i])
               {
                    mixer.PlaySound(sound[i]);
                    mixer.startNativeAudio();
               }
          }
          
//          contOfPlaying = mixer.playingSound();
          SDL_Delay(100);
          ++timer;
     } while ( mixer.timing().secLeft );
          
     SDL_Delay(10);
     mixer.closeNativeAudio();
}
