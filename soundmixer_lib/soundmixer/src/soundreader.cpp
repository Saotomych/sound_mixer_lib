#include <soundmixer/src/soundreader.h>
#include <soundmixer/wav_header.h>

#include <algorithm>
#include <utility>

using namespace soundreader;

// Length of the time slot of the mixed playing in seconds
constexpr double timeSlotLen = 0.1;

SoundReader::SoundReader(int32_t handle): soundHandle(handle)
{
}

SoundReader::~SoundReader()
{
    PlatformCloseSoundFile(soundHandle);
}

bool SoundReader::Open(std::string filename, int32_t handle)
{
    handle = PlatformOpenSoundFile(filename.c_str(), handle);
    if (handle != INVALID_HANDLE)
    {
        bool res = PlatformReadWavHeader(handle, wHeader);
        if (!res)
            return false;

        // set vector sizes for 0,1 sec of playing
        uint32_t sz = wHeader.byteRate * timeSlotLen;
        uint32_t bapc = wHeader.blockAlign / wHeader.numChannels;
        blockLen = sz / bapc * bapc;
        bytesLeft = wHeader.subchunk2Size;
        seconds =  (double)(bytesLeft) / wHeader.byteRate;
        if ( bufFromFile.size() < blockLen )
            bufFromFile.resize(blockLen);

        return true;
    }

    return false;
}

bool SoundReader::NextDataBlock(uint8_t*& buffer, uint32_t& size)
{
    if (blockLen)
    {
        uint32_t length = bytesLeft;
        length = (length < blockLen) ? length : blockLen;
        if (length)
        {
            PlatformReadFile(soundHandle, bufFromFile.data(), length);
            bytesLeft -= length;
        } else {
            length = 8;
            bufFromFile.resize(length);
            std::fill(bufFromFile.begin(), bufFromFile.end(), bufFromFile.size());
        }

        size = length;
        buffer = (uint8_t*) bufFromFile.data();
        return true;
    }

    return false;
}

double SoundReader::FullTime() const
{
    return seconds;
}

uint32_t SoundReader::SizeLeft() const
{
    return bytesLeft;
}

const WavHeader& SoundReader::Header() const
{
    return wHeader;
}
