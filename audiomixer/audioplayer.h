#ifndef AUDIO_PLAYER_H
#define AUDIO_PLAYER_H

#define INVALID_HANDLE (-1)

#include "audiosets.h"

namespace audioreader
{
     class AudioReader;
}

namespace audiomixer
{
     class AudioMixer;
}

// Кросс-Платформенный интерфейс
namespace audioplayer
{

// На десктопе реализация через PLATFORM_SDL_H
// На чибике реализация через DMA UART
// Они требуют совсем разных описаний для инициализации
// Устройство для инициализации всегда одно: хотя физически их может быть несколько
     
// Фейковый заголовок должен создаваться на уровне платформозависимого кода     
     
     bool openDevice(audioreader::AudioReader* reader, audiomixer::AudioMixer* mixer);
     
     void closeDevice();
     
     void startPlay();
     
     void pausePlay();
     
     void breakPlay();
     
     void delayPlay( uint32_t ms );
}

#endif // AUDIO_PLAYER_H