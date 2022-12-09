#include <include/audiomixer_api.h>
#include <include/audio_platform.h>
#include <audiomixer/wav_header.h>
#include <audiomixer/src/audiomixer.h>
#include <stdexcept>

using namespace audiomixer;

AudioMixerApi::AudioMixerApi(): mixer(std::make_unique<AudioMixer>())
{
}

AudioMixerApi::~AudioMixerApi()
{
}

int32_t AudioMixerApi::PlaySound(std::string fileName)
{
    int32_t soundHandle = mixer->AddSound(fileName);
    if (soundHandle == INVALID_HANDLE)
    {
        throw std::invalid_argument("File name not found");
    }

    return soundHandle;
}

void AudioMixerApi::StopSound(int32_t handle) noexcept
{
    mixer->RemoveSound(handle);
}

void AudioMixerApi::PauseSound(int32_t) noexcept
{

}

void AudioMixerApi::ResumeSound(int32_t) noexcept
{

}

void AudioMixerApi::Pause() noexcept
{
    PlatformPausePlay();
}

void AudioMixerApi::Resume() noexcept
{
    PlatformStartPlay();
}

double AudioMixerApi::LeftTime() noexcept
{
    return mixer->SecLeft();
}

uint64_t AudioMixerApi::LeftSize() noexcept
{
    return mixer->SizeLeft();
}

void AudioMixerApi::DelayPlay(uint32_t ms) noexcept
{
    PlatformDelayPlay(ms);
}

void AudioMixerApi::BreakPlay() noexcept
{
    PlatformBreakPlay();
}
