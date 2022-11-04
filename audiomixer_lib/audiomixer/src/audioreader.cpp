#include <audiomixer/src/audioreader.h>
#include <audiomixer/wav_header.h>

#include <algorithm>
#include <utility>

using namespace audioreader;

AudioReader::AudioReader(int32_t handle): soundHandle(handle)
{
}

AudioReader::~AudioReader()
{
    PlatformCloseAudioFile(soundHandle);
}

bool AudioReader::Open(std::string filename, int32_t handle)
{
    handle = PlatformOpenAudioFile(filename.c_str(), handle);
    if (handle != INVALID_HANDLE)
    {
        bool res = PlatformReadWavHeader(handle, wHeader);
        if (!res)
            return false;

        // set vector sizes for 0,1 sec of playing
        uint32_t sz = wHeader.byteRate * 0.1;
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

bool AudioReader::NextDataBlock(uint8_t*& buffer, uint32_t& size)
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

double AudioReader::FullTime() const
{
    return seconds;
}

uint32_t AudioReader::SizeLeft() const
{
    return bytesLeft;
}

const WavHeader& AudioReader::Header() const
{
    return wHeader;
}
