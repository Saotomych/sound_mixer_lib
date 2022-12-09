#include <include/audiomixer_api.h>
#include <include/audio_platform.h>

#include <vector>
#include <unordered_map>
#include <algorithm>
#include <iostream>

#include <gtest/gtest.h>

namespace
{

// when sounds like each others
std::unordered_map<std::string, std::vector<uint8_t>> testDataSet1 = {
    { "data-1", { (uint8_t)-30, 30, 20, 30, 40, 20 }},
    { "data-2", { (uint8_t)-30, 30, 40 }},
    { "data-3", { (uint8_t)-30, 30, 30, 40, 30, 30 }},
    { "data-4", { (uint8_t)-30, 30,  0, 20, 20, 40 }},
    { "expected", { (uint8_t)-94, 94, 78, 78, 78, 78 }}
};

// when one sound has reverted phase
std::unordered_map<std::string, std::vector<uint8_t>> testDataSet2 = {
    { "data-1", { (uint8_t)-1, 0x7F }},
    { "data-2", { (uint8_t)-1, 0x7F }},
    { "data-3", { 0x03, (uint8_t)-127 }},
    { "data-4", { (uint8_t)-1, 0x7F }},
    { "expected", { 0, 0xFE }}
};

// when 2 sounds have reverted phase
std::unordered_map<std::string, std::vector<uint8_t>> testDataSet3 = {
    { "data-1", { (uint8_t)-1, (uint8_t)-127 }},
    { "data-2", { (uint8_t)-1, 0x7F }},
    { "data-3", { 0x03, (uint8_t)-127 }},
    { "data-4", { (uint8_t)-1, 0x7F }},
    { "expected", { 0, 0 }}
};

// Environment of the callbacks from the soundmixer

// Call the callback to fill mixedResult
cbFn userFn = nullptr;
// with the user context
void* userCtx = nullptr;

uint16_t blockAlign;
uint16_t bitsPerSample;
std::unordered_map<int32_t, std::vector<uint8_t> > rawStreams;
std::unordered_map<int32_t, uint32_t > streamSizes;
std::reference_wrapper<
std::unordered_map<std::string, std::vector<uint8_t>>> testSet = testDataSet1;

// Set of the callbacks from the soundmixer
extern "C"
uint32_t PlatformFileSizeLeft(int32_t handle)
{
    return streamSizes[handle];
}

extern "C"
int32_t PlatformOpenAudioFile(const char* fileName, int32_t handle)
{
    rawStreams[handle] = testSet.get()[fileName];
    streamSizes[handle] = rawStreams[handle].size();
    return handle;
}

extern "C"
void PlatformCloseAudioFile(int32_t handle)
{
    rawStreams.erase(handle);
}

extern "C"
uint32_t PlatformReadFile(int32_t handle, char* buffer, uint32_t size)
{
    auto& data = rawStreams[handle];
    uint32_t leftSize = streamSizes[handle];
    uint32_t copySize = std::min(leftSize, size);
    std::copy(&data[0], &data[copySize], buffer);
    streamSizes[handle] -= copySize;
    return copySize;
}

extern "C"
bool PlatformReadWavHeader(int32_t handle, WavHeader& header)
{
    header.numChannels = 1;
    header.sampleRate = 11025;
    header.blockAlign = blockAlign;
    header.bitsPerSample = bitsPerSample;
    header.byteRate = 11025;
    header.blockAlign = 1;
    header.subchunk2Size = streamSizes[handle];
    header.chunkSize = streamSizes[handle];
    header.subchunk1Size = streamSizes[handle];

    return true;
}

extern "C"
bool PlatformOpenDevice(const WavHeader*, cbFn fn, void *cbUserCtx)
{
    userFn = fn;
    userCtx = cbUserCtx;
    return true;
}

extern "C"
void PlatformCloseDevice(){}

extern "C"
void PlatformStartPlay(){}

extern "C"
void PlatformPausePlay(){}

extern "C"
void PlatformBreakPlay(){}

extern "C"
void PlatformDelayPlay(uint32_t){}

} // namespace

struct TestSet
{
    uint16_t blockAlign;
    uint16_t bitsPerSample;
    std::unordered_map<std::string, std::vector<uint8_t>>& dataSet;
};

TestSet testSet1 = { 1, 8, testDataSet1 };
TestSet testSet2 = { 2, 16, testDataSet2 };
TestSet testSet3 = { 2, 16, testDataSet3 };

class MixingTest : public ::testing::TestWithParam<TestSet> 
{
public:

    virtual ~MixingTest() = default;

    void SetUp() override {}
    void TearDown() override {}

    std::vector<uint8_t> mixedResult;

};

TEST_P(MixingTest, Mix4DataBlocks)
{
    using namespace audiomixer;

    TestSet set = GetParam();
    blockAlign = set.blockAlign;
    bitsPerSample = set.bitsPerSample;
    testSet = set.dataSet;

    AudioMixerApi sndMix;
    int32_t hndl[4];
    EXPECT_NO_THROW(hndl[0] = sndMix.PlaySound("data-1"));
    EXPECT_NO_THROW(hndl[1] = sndMix.PlaySound("data-2"));
    EXPECT_NO_THROW(hndl[2] = sndMix.PlaySound("data-3"));
    EXPECT_NO_THROW(hndl[3] = sndMix.PlaySound("data-4"));
    EXPECT_EQ(0, hndl[0]);
    EXPECT_EQ(1, hndl[1]);
    EXPECT_EQ(2, hndl[2]);
    EXPECT_EQ(3, hndl[3]);
    mixedResult.resize(sndMix.LeftSize());
    userFn(userCtx, mixedResult.data(), sndMix.LeftSize());

    EXPECT_EQ(testSet.get()["expected"], mixedResult);
}

INSTANTIATE_TEST_SUITE_P(SoundMixerTest, MixingTest, 
    testing::Values(testSet1, testSet2, testSet3));
