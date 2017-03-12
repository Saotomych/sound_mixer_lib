#include <audiomixer/audiomixer_api.h>
#include <audiomixer/src/audiomixer.h>
#include <stdexcept>

AudioMixerApi::AudioMixerApi(): mixer(std::make_unique<Mixer())
{
}

AudioMixerApi::~AudioMixerApi()
{
}

int32_t AudioMixerApi::PlaySound(std::string fileName)
{
    int32_t soundHandle = mixer.addSound( fileName );
    if ( soundHandle == INVALID_HANDLE )
    {
        throw std::invalid_argument("File name not found");
    }

    return soundHandle;
}

void AudioMixerApi::StopSound(int32_t handle)
{
    mixer.removeSound(handle);
}

void AudioMixerApi::PauseSound(int32_t handle)
{

}

void AudioMixerApi::ResumeSound(int32_t handle)
{

}

void AudioMixerApi::Pause()
{
    audioplayer::pausePlay();
}

void AudioMixerApi::Resume()
{
    audioplayer::startPlay();
}

double AudioMixerApi::LeftTime()
{
    return mixer.secLeft();
}

uint32_t AudioMixerApi::LeftSize()
{
    return mixer.sizeLeft();
}
