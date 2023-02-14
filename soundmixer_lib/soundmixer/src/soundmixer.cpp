#include <include/platform_deps.h>
#include <include/soundmixer_api.h>
#include <soundmixer/src/soundmixer.h>
#include <soundmixer/wav_header.h>

#include <algorithm>
#include <utility>
#include <memory>
#include <cassert>
#include <memory>

#ifdef MULTITHREADING_ON
#define LOCAL_LOCK_GUARD(Type, Mutex) std::lock_guard<Type> lock(Mutex);
#else
#define LOCAL_LOCK_GUARD(Type, SyncM)
#endif

namespace
{
    uint32_t platformCbFn(void* udata, uint8_t* buffer, int size)
    {
        soundmixer::SoundMixer* mixer = reinterpret_cast<soundmixer::SoundMixer*>(udata);
        return mixer->GetMixedBuffer(buffer, size);
    }
}

using namespace soundmixer;

uint32_t SoundMixer::GetMixedBuffer(uint8_t* mixedData, uint32_t length)
{
    if (!mixedData)
        return 0;

    LOCAL_LOCK_GUARD(std::mutex, mtSoundReaders);
    uint8_t* soundBuffer = nullptr;
    uint32_t soundSize = 0;
    uint32_t maxSize = soundSize;

    std::fill(mixedData, &mixedData[length], 0);
    for (auto& reader: readers)
    {
        if (!reader.second.NextDataBlock(soundBuffer, soundSize) || 
            !soundSize || soundSize > length || !soundBuffer) {
            continue;
        }

        if (maxSize < soundSize)
            maxSize = soundSize;

        uint8_t* dt = soundBuffer;
        uint8_t* mx = mixedData;
        for (uint32_t j = 0; j < soundSize; j = j + bapc)
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
        for (const auto &reader: readers) {
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

int32_t SoundMixer::AddSound(std::string fileName)
{
    LOCAL_LOCK_GUARD(std::mutex, mtSoundReaders);

    auto it = readers.emplace(nextHandle, soundreader::SoundReader(nextHandle));
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

void SoundMixer::RemoveSound(int32_t handle)
{
    LOCAL_LOCK_GUARD(std::mutex, mtSoundReaders);
    DeleteSound(handle);
}

double SoundMixer::SecLeft()
{
    LOCAL_LOCK_GUARD(std::mutex, mtSoundReaders);
    return fullSecondLeft;
}

uint32_t SoundMixer::SizeLeft()
{
    LOCAL_LOCK_GUARD(std::mutex, mtSoundReaders);
    return fullSizeLeft;
}

int32_t SoundMixer::TakeValue(uint8_t* buf)
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

void SoundMixer::PutValue(uint8_t* buf, int32_t value)
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

void SoundMixer::DeleteSound(uint32_t idx)
{
    readers.erase(idx);
}
