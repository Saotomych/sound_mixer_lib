#ifndef AUDIO_API_H
#define AUDIO_API

#include <iostream>

#include "audiomixer.h"

class AudioApi
{
     
     audiomixer::AudioMixer mixer;
     
public:
     
     AudioApi() {}
     
     int32_t playSound(std::string fileName)
     {
          int32_t soundHandle = mixer.addSound( fileName );
          
          if ( soundHandle != INVALID_HANDLE )
          {
               std::cout << "Play begin: " << std::endl
               << fileName << " is left " << mixer.secLeft() << " seconds and " << mixer.sizeLeft() << " bytes." << std::endl;
          }
          else
          {
               std::cout << "File not found" << std::endl;
          }
          
          return soundHandle;
     }
     
     void stopSound(int32_t handle)
     {
          mixer.removeSound(handle);
     }
     
     void pauseSound(int32_t handle)
     {
          
     }
     
     void resumeSound(int32_t handle)
     {
          
     }
     
     void pause()
     {
          audioplayer::pausePlay();
     }
     
     void resume()
     {
          audioplayer::startPlay();
     }

     double secondsLeft()
     {
          return mixer.secLeft();
     }

     uint32_t sizeLeft()
     {
          return mixer.sizeLeft();
     }
};

#endif // AUDIO_API