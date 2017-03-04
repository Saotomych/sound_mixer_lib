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

#include <SDL2/SDL.h>
#include "sdl/include/audio.h"

#include "wavheader.h"

#define NODEBUG		0
#define DBGLEVEL1 	1
#define DBGLEVEL2 	2
#define DBGLEVEL3 	3

#define DEBUG	NODEBUG

using namespace std;

// prototype for our audio callback
// see the implementation for more information
void my_audio_callback(void *userdata, Uint8 *stream, int len)
{
    cout << "callback" << endl;
}

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
        for (int i = 0; i < bufflen; ++i) {
            int max_val_square = 1073741824; 
	    // нaша скорость света -> max short ^ 2 => 32768*32768  (ибо я работаю с 16bit-ным(знаковым) звуком)
            float a = buf1[i];
            float b = buf2[i];
            float rel_samp = (a + b) / (1 + (a * b) / max_val_square);
            out_buf[i] = rel_samp;
        }
*/        
        
class WavInfo
{
  vector<char> buf1;
  vector<char> buf2;

  char* rdBuf;
  char* wrBuf;
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
	while ( sz % bapc ) 
	  ++sz;
	
	blockLen = sz;
	buf1.resize(sz);
	buf2.resize(sz);
	rdBuf = buf1.data();
	wrBuf = buf2.data();
	
	bytesLeft = wHeader.subchunk2Size;
	secondsLeft =  (double)(bytesLeft) / wHeader.byteRate;
	
	uint8_t* fakebuf;
	uint32_t fakeSize; 
	nextDataBlock(fakebuf, fakeSize);

	return (wavStream.rdstate()) ? false : true;
    }
    
    return false;
  }

  void clear() 
  {
    if (wavStream.is_open())
	wavStream.close();
  }
  
// @brief returns as parameters: pointer to sound data buffer
  bool nextDataBlock(uint8_t*& buffer, uint32_t& size)
  {
      size = 0;
      
      if (blockLen)
      {
	  buffer = (uint8_t*)wrBuf;
	  size = lastBlockSize;
	
	  int length = wHeader.chunkSize - wavStream.tellg() + 8;
	  length = (length < blockLen) ? length : blockLen;

	  if (length)
	    wavStream.read(rdBuf, length);
	  
	  lastBlockSize = length;
	  
	  assert(!(wavStream.rdstate() & ifstream::failbit));
	  assert(!(wavStream.rdstate() & ifstream::badbit));
	  
	  swap(rdBuf, wrBuf);
	  
	  bytesLeft -= size;
	  secondsLeft = (double)(bytesLeft) / wHeader.byteRate;
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
    uint8_t mixedData[16384];
    uint8_t mixedSize[16384];
    uint16_t ba;
    uint16_t bapc;
    uint16_t bps;
    int64_t valueLimit;
    int64_t vl;
     
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

    inline
    uint32_t nearly_sqrt(int32_t value)
    {
	    value = abs(value);
	    int32_t vsdv = value;
	    int32_t v = 2;

	    if (value == 0)
	      return 0;
	    
	    if (value >= (1 << 16) )
	    {
		    vsdv >>= 8;
		    v = 512;		
	    }

	    if (value >= (1 << 22) )
	    {
		    vsdv >>= 3;
		    v = 4096;	
	    }

	    while ( v < vsdv )
	    {
		    v <<= 1;
		    vsdv >>= 1;
	    }
	    
	    return ( vsdv + value / vsdv ) / 2;
    }
    
public:
  
    MixedSound(uint16_t blockAlign, uint16_t bitsPerSample): 
      ba(blockAlign), bps(bitsPerSample), index(0) 
    {
	bapc = ba;		// stereo has only one channel here
	valueLimit = (1 << (bps-1)) - 1;
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
		int64_t base = (int32_t) takeValue(mx);
		int64_t adder = (int32_t) takeValue(dt);
		int64_t val = (base + adder) * vl / ( vl + abs(base * adder) / vl );
		
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

  MixedSound* mixedSound;
  
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
  
public:
  
  SndMixer(): empty(-1), soundHandle(-1), t(), mixedSound(nullptr) {}
  
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
	 delete mixedSound;
	 return false;
      }

      // when global time left equals 0 new wave header must generate
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
	    generateWavHeader(hndl);
      }
      
      // Global time counters refresh
      t.secPlayed += 0.1;
      t.secLeft -= 0.1;

      // Preparing of sound buffers
      vector<uint8_t*> data;
      vector<uint32_t> size;
      uint32_t maxsize = 0;

      bool nextRdy = false;
      for(WavInfo& v: sounds)
      {
	  uint8_t* d;
	  uint32_t s;
	  nextRdy |= v.nextDataBlock(d, s);
	  if (s)
	  {
	    data.push_back(d);
	    size.push_back(s);
	  }
	  
	  if ( s > maxsize )
	    maxsize = s;
	  
	  // Time detector
	  cout << v.name() << " is left " << v.secLeft() << " seconds and " << v.sizeLeft() << " bytes." << endl;
//	  cout << "Playing size = " << s << endl;
      }

      // TODO Mix sound buffers
      assert(mixedSound);
      mixedSound->mix(data, size, maxsize);
      
      // TODO: play mixed sound with DMA
	  // Wait of hardware
	  // Start

      if (nextRdy == false)
	 delete mixedSound;
	  
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
	generateWavHeader(info);
	mixedSound = new MixedSound(info.header().blockAlign, info.header().bitsPerSample);
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

int main()
{
    // Mixed sound tests
  cout << ">>> new test " << endl;
  clipMixedTest1byte();
  cout << ">>> new test " << endl;
  clipMixedTest3byte1();
  cout << ">>> new test " << endl;
  clipMixedTest3byte2();
  
//   cout << "new test " << endl;
//   clipMixedTest3bytea();

   /* Initialize only SDL Audio on default device */
//     if(SDL_Init(SDL_INIT_AUDIO) < 0)
//     {
//         return 1; 
//     }
// 
//     /* Init Simple-SDL2-Audio */
//     initAudio();
// 
//     /* Play music and a sound */
//     playSound("romans.wav", SDL_MIX_MAXVOLUME);
// //    playMusic("falling16bit.wav", SDL_MIX_MAXVOLUME);
// 
//     playSound("cow.wav", SDL_MIX_MAXVOLUME);
//     playSound("falling16bit.wav", SDL_MIX_MAXVOLUME);
// 
//     SDL_Delay(8000);
// 
//     /* End Simple-SDL2-Audio */
//     endAudio();
// 
//     SDL_Quit();

  return 0;

    vector<string> sound = { "cow.wav", "falling16bit.wav", "romans.wav" };
    vector<int> startTime = { 0, 5, 30 }; // with 0,1 second step
  
    SndMixer mixer;

    int timer = 0;
    bool contOfPlaying = true;
    while ( contOfPlaying )
    {
      
	std::cout.precision(2);
	cout << "- !new step! - Timer: " << ((double)(timer)/10) << endl;
	std::cout.precision(5);
      
	for (int i=0; i < sound.size(); ++i) 
	{
	  if (timer == startTime[i])
	    mixer.PlaySound(sound[i]);
	}
	  
	contOfPlaying = mixer.playingSound();
	++timer;
    }
    
}
