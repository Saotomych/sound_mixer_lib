#include <include/soundmixer_api.h>
#include <include/platform_deps.h>
#include <soundmixer/wav_header.h>
#include <soundmixer/src/soundmixer.h>
#include <stdexcept>

using namespace soundmixer;

SoundMixerApi::SoundMixerApi(): mixer(std::make_unique<SoundMixer>())
{
}

SoundMixerApi::~SoundMixerApi()
{
}

int32_t SoundMixerApi::PlaySound(std::string fileName)
{
    int32_t soundHandle = mixer->AddSound(fileName);
    if (soundHandle == INVALID_HANDLE)
    {
        throw std::invalid_argument("File name not found");
    }

    return soundHandle;
}

void SoundMixerApi::StopSound(int32_t handle) noexcept
{
    mixer->RemoveSound(handle);
}

void SoundMixerApi::Pause() noexcept
{
    PlatformPausePlay();
}

void SoundMixerApi::Resume() noexcept
{
    PlatformStartPlay();
}

double SoundMixerApi::LeftTime() noexcept
{
    return mixer->SecLeft();
}

uint64_t SoundMixerApi::LeftSize() noexcept
{
    return mixer->SizeLeft();
}

void SoundMixerApi::DelayPlay(uint32_t ms) noexcept
{
    PlatformDelayPlay(ms);
}

void SoundMixerApi::BreakPlay() noexcept
{
    PlatformBreakPlay();
}
