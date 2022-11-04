#include <audiomixer/src/audiomixer.h>
#include <audiomixer/wav_header.h>
#include <audio_platform/audio_platform.h>

#include <algorithm>
#include <utility>
#include <memory>
#include <cassert>
#include <memory>

namespace
{
    uint32_t platformCbFn(void* udata, uint8_t* buffer, int size)
    {
        audiomixer::AudioMixer* mixer = reinterpret_cast<audiomixer::AudioMixer *>(udata);
        return mixer->GetMixedBuffer(buffer, size);
    }
}

using namespace audiomixer;

uint32_t AudioMixer::GetMixedBuffer(uint8_t* mixedData, uint32_t length)
{
    assert(mixedData != nullptr);

    std::unique_lock<std::mutex> lock( mtSoundReaders );
    uint8_t* audioBuffer = nullptr;
    uint32_t audioSize = 0;
    uint32_t maxSize = audioSize;

    std::fill(mixedData, &mixedData[length], 0);
    for (auto& reader: readers)
    {
        if (!reader.second.NextDataBlock(audioBuffer, audioSize) || audioSize == 0) {
            continue;
        }

        assert(audioSize <= length);
        assert(audioBuffer != nullptr);

        if (maxSize < audioSize)
            maxSize = audioSize;

        uint8_t* dt = audioBuffer;
        uint8_t* mx = mixedData;
        for (uint32_t j = 0; j < audioSize; j = j + bapc)
        {
            int32_t base = (int32_t) TakeValue(mx);
            int32_t adder = (int32_t) TakeValue(dt);
            int32_t val = (base + adder) * vl / ( vl + abs(base * adder) / vl );

            PutValue(mx, val);

            dt += bapc;
            mx += bapc;
        }
    }

    bool pass = false;
    while(!pass) {
        pass = true;
        for (auto &reader: readers) {
            if (!reader.second.SizeLeft()) {
                readers.erase(reader.first);
                pass = false;
                break;
            }
        }
    }

    fullSecondLeft -= (double) maxSize / btrate;
    if (abs(fullSecondLeft) < 0.01)
        fullSecondLeft = 0.;

    fullSizeLeft -= maxSize;

    return maxSize;
}

int32_t AudioMixer::AddSound(std::string fileName)
{
    std::unique_lock<std::mutex> lock(mtSoundReaders);

    auto it = readers.emplace(nextHandle, audioreader::AudioReader(nextHandle));
    if (it.second && it.first->second.Open(fileName, nextHandle))
    {
        auto& ar = it.first->second;
        const WavHeader& wh = ar.Header();
        if (fullSecondLeft == 0.)
        {
            if (PlatformOpenDevice(&wh, platformCbFn, this))
            {
                PlatformStartPlay();
            } else {
                readers.erase(it.first);
                return INVALID_HANDLE;
            }
        }

        if (fullSizeLeft < ar.SizeLeft())
        {
            fullSecondLeft = ar.FullTime();
            fullSizeLeft = ar.SizeLeft();
        }

        ba = wh.blockAlign;
        bps = wh.bitsPerSample;
        bapc = ba;          // assumption: stereo has only one channel here
        vl = (1 << (bps-1)) - 1;
        btrate = wh.byteRate;
        auto tmp = nextHandle++;
        return tmp;
    }

    if (it.second)
        readers.erase(it.first);

    return INVALID_HANDLE;
}

void AudioMixer::RemoveSound(int32_t handle)
{
    std::unique_lock<std::mutex> lock(mtSoundReaders);
    DeleteSound(handle);
}

double AudioMixer::SecLeft()
{
    std::unique_lock<std::mutex> lock(mtSoundReaders);
    return fullSecondLeft;
}

uint32_t AudioMixer::SizeLeft()
{
    std::unique_lock<std::mutex> lock(mtSoundReaders);
    return fullSizeLeft;
}

int32_t AudioMixer::TakeValue(uint8_t* buf)
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

void AudioMixer::PutValue(uint8_t* buf, int32_t value)
{
    uint8_t* pVal = (uint8_t*) (&value);
    for (uint8_t i = 0; i < bapc; ++i)
    {
        *buf = *pVal;
        ++buf;
        ++pVal;
    }

    if (value < 0)
    {
        *(buf-1) |= 0x80;
    }
}

void AudioMixer::DeleteSound(uint32_t idx)
{
    readers.erase(idx);
}
