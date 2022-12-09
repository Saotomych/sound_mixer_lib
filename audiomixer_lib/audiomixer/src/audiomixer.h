#ifndef AUDIO_MIXER_H
#define AUDIO_MIXER_H

#include <string>
#include <mutex>
#include <unordered_map>

#include <audiomixer/src/audioreader.h>

namespace audiomixer
{

class AudioMixer
{
public:
     
     AudioMixer(): nextHandle(0), fullSecondLeft(0.), fullSizeLeft(0)
     {}
     
     uint32_t GetMixedBuffer(uint8_t* mixedData, uint32_t length);

     int32_t AddSound(std::string fileName);

     void RemoveSound(int32_t handle);

     double SecLeft();

     uint32_t SizeLeft();

private:
    std::unordered_map<uint32_t, audioreader::AudioReader> readers;
    std::mutex mtSoundReaders;

    uint16_t ba;   // block align
    uint16_t bapc; // bit per channel
    uint16_t bps;  // bit per sample
    double btrate; // timing per second
    int32_t vl;
    int32_t nextHandle = 0;
    double fullSecondLeft = 0.;
    uint32_t fullSizeLeft = 0;

    int32_t TakeValue(uint8_t* buf);
    void PutValue(uint8_t* buf, int32_t value);
    void DeleteSound(uint32_t idx);

};

} // audiomixer

#endif // AUDIO_MIXER_H
