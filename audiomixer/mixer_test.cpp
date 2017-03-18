#include <vector>
#include <string>

#include "audioapi.h"

using namespace std;

int main()
{
     
     vector<string> sound = { 
//          "./audio/road16bit11025.wav", 
          "./audio/cow.wav", 
          "./audio/romans.wav", 
          "./audio/falling16bit.wav", 
          "./audio/cow.wav", 
          "./audio/romans.wav", 
          "./audio/falling16bit.wav", 
          "./audio/cow.wav", 
          "./audio/romans.wav", 
     };
     
//      vector<string> sound = { "highlands16bit.wav", "silence.wav", "silence.wav", "silence.wav", "silence.wav",
//           "silence.wav", "silence.wav", "silence.wav", "silence.wav"
//      };
     
     vector<int> startTime = { 0, 5, 12, 14, 19, 20, 25, 32, 35 }; // with 0,1 second step
     
     AudioApi sndApi;
     
     int timer = 0;
     double lastTime = 0.;
     std::cout << std::fixed;
     do{

          for (uint32_t i=0; i < sound.size(); ++i) 
          {
               if (timer == startTime[i])
               {
                    std::cout << "start " << i << " sound" << std::endl;
                    sndApi.playSound(sound[i]);
                    
                    if ( lastTime < sndApi.secondsLeft() )
                         lastTime = sndApi.secondsLeft();
               }
          }

          std::cout.precision(2);
          cout << "- !new step! - Timer: " << ((double)(timer)/10) << "; left " << lastTime << " secs" << endl;
          std::cout.precision(5);
          
          audioplayer::delayPlay( 100 );
          ++timer;
          lastTime -= 0.1;
     } while ( lastTime > 0 );
     
     audioplayer::delayPlay( 100 );

     audioplayer::breakPlay();
}
