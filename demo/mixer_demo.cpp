#include <vector>
#include <string>
#include <iostream>

#include <audiomixer_lib/include/audiomixer_api.h>

int main()
{
     
//      vector<string> sound = { 
//           "./audio/road16bit11025.wav", 
//           "./audio/cow.wav", 
//           "./audio/romans.wav", 
//           "./audio/falling16bit.wav", 
//           "./audio/cow.wav", 
//           "./audio/romans.wav", 
//           "./audio/falling16bit.wav", 
//           "./audio/cow.wav", 
//           "./audio/romans.wav", 
//      };
     
     std::vector<std::string> sound = { "./audio/test_sound1.wav",
                                        "./audio/test_sound2.wav",
                                        "./audio/test_sound3.wav",
                                        "./audio/test_sound4.wav",
                                        "./audio/test_sound5.wav",
                                        "./audio/test_sound6.wav",
                                        "./audio/test_sound7.wav",
                                        "./audio/test_sound8.wav",
                                        "./audio/test_sound9.wav",
                                        "./audio/test_sound10.wav",
                                        "./audio/test_sound11.wav",
                                        "./audio/test_sound12.wav",
                                        "./audio/test_sound13.wav",
                                        "./audio/test_sound14.wav",
                                        "./audio/test_sound15.wav",
     };
     
     std::vector<int> startTime = {
             0, 5, 120, 140, 190,
             200, 250, 320, 350, 400,
             450, 510, 600, 640, 750
     }; // with 0,1 second step

     audiomixer::AudioMixerApi sndMixer;

     int timer = 0;
     double lastTime = 0.;

     std::cout << std::fixed;

     do{

          for (uint32_t i=0; i < sound.size(); ++i) 
          {
               if (timer == startTime[i])
               {
                    std::cout << "start " << i << " sound" << std::endl;
                    sndMixer.PlaySound(sound[i]);
                    
                    if ( lastTime < sndMixer.LeftTime() )
                         lastTime = sndMixer.LeftTime();
               }
          }

          std::cout.precision(2);
          std::cout << "- !new step! - Timer: " << ((double)(timer)/10) << "; left " << lastTime << " secs" << std::endl;
          std::cout.precision(5);

          sndMixer.DelayPlay(100);
          ++timer;
          lastTime -= 0.1;
     } while ( lastTime > 0 );

     sndMixer.DelayPlay(100);
     sndMixer.BreakPlay();
}
