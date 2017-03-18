#include <audiomixer/src/audioreader.h>

using namespace audioreader;

AudioReader::AudioReader(int32_t handle): soundHandle(handle)
{
}

AudioReader::~AudioReader()
{
    audioreader::closeAudioFile( soundHandle );
}

bool AudioReader::Open(std::string filename, int32_t handle)
{
    handle = audioreader::openAudioFile(filename.c_str(), handle);
    if ( handle != INVALID_HANDLE )
    {
        bool res = audioreader::readWavHeader(handle, wHeader);
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
            audioreader::readFile(soundHandle, bufFromFile.data(), length);
        }
        size = length;
        buffer = (uint8_t*) bufFromFile.data();
        bytesLeft -= length;
        return true;
    }

    return false;
}

double AudioReader::FullTime()
{
    return seconds;
}

uint32_t AudioReader::SizeLeft()
{
    return bytesLeft;
}

const WavHeader& AudioReader::Header()
{
    return wHeader;
}