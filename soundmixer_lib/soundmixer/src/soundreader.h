#ifndef SOUND_READER_H
#define SOUND_READER_H

#include <soundmixer/wav_header.h>
#include <include/platform_deps.h>

#include <vector>
#include <string>

namespace soundreader
{

class SoundReader
{
public:

     /// @brief ctor
     /// @param [in] handle of the definite sound
     SoundReader(int32_t handle);

     /// @brief dtor
     ~SoundReader();

     /// @brief Open wav file
     /// @param [in] filename Name of wav file
     /// @param [in] handle of the definite sound
     /// @return true when file has opened
     bool Open(std::string filename, int32_t handle);

     /// @brief Get the next block of the sound data
     /// @param [out] buffer Pointer to a sound data buffer
     /// @param [out] size Size of the buffer
     bool NextDataBlock(uint8_t*& buffer, uint32_t& size);

     /// @brief Full sound time in seconds
     /// @return sound time in seconds
     double FullTime() const;

     /// @brief Size to end
     /// @return size to end in bytes
     uint32_t SizeLeft() const;

     ///  @brief Get Wav header
     /// @return Wav header
     const WavHeader& Header() const;

private:
    std::vector<char> bufFromFile;
    uint32_t blockLen = 0;
    WavHeader wHeader;
    int32_t soundHandle = INVALID_HANDLE;
    double seconds = 0.;
    uint32_t bytesLeft = 0;
};

} // soundreader

#endif // SOUND_READER_H
