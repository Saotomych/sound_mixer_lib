#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sys/stat.h>

#include <audiomixer_lib/include/audiomixer_api.h>
#include <demo/cl_parser.hpp>

#include <rapidjson/document.h>

struct Cli
{
    std::string directory;
    std::string sndFile;
    std::string cfgFile;

    bool check() const
    {
        uint8_t cnt;
        if (directory.size())
            cnt++;
        if (sndFile.size())
            cnt++;
        if (cfgFile.size())
            cnt++;
        if (cnt == 1)
            return true;
        return false;
    };
};

Cli parseCommandLine(int argc, char** argv)
{
    Cli cli;
    bool printHelp = false;
    CommandLine args("A demonstration of the sound mixer library with SDL 2.0 platform."
                     "Only one of all options must be set");
    args.addArgument({"-d", "--directory"}, &cli.directory, "The directory where sound files are placed.\n"
                    "Use this key when you want to play all sounds in directory simultaneously.");
    args.addArgument({"-f", "--file"}, &cli.sndFile, "Play the sound alone.");
    args.addArgument({"-c", "--config"}, &cli.cfgFile, "Play files from config file. There are more rich settings than "
                                                  " in the command line.\n"
                                                  "JSON format:\n"
                                                  "directory:{<base directory>} - directory where spund files "
                                                  "are placed.\n"
                                                  "files:{<filenames of sounds>} - array of the filenames "
                                                  "for playing.\n"
                                                  "delays:{<delays of the start certain sound regarding "
                                                  "the start of application.");
    args.addArgument({"-h", "--help"}, &printHelp,
                    "Print this help. This help message is actually so long "
                    "that it requires a line break!");

    args.parse(argc, argv);

     if (printHelp) {
          args.printHelp();
          throw std::runtime_error("Help printed successfully.");
     }
     return cli;
}

bool parseConfigFile(
    const std::string& cfgFile,
    std::reference_wrapper<std::vector<std::string>> _sounds,
    std::reference_wrapper<std::vector<uint32_t>> _delays)
{
    auto& sounds = _sounds.get();
    auto& delays = _delays.get();

    if (cfgFile.empty())
        return false;

    std::vector<wchar_t> jsondoc;
    std::ios_base::sync_with_stdio(false);
    std::wcout.imbue(std::locale("C.UTF-8"));
    std::wifstream cfg(cfgFile, std::ios::binary);
    if (!cfg.is_open()) 
    {
        std::cout << "File " << cfgFile << " not found." << std::endl;
        return false;
    }
    cfg.imbue(std::locale("C.UTF-8"));
    std::copy(std::istreambuf_iterator<wchar_t>(cfg), std::istreambuf_iterator<wchar_t>(), 
         std::back_inserter(jsondoc));

    // TODO Fill sounds and delays from json
    std::copy(jsondoc.begin(), jsondoc.end(), std::ostream_iterator<wchar_t, wchar_t>(std::wcout, L""));    
    return true;
}

bool fillConfigFromDirectory(
    const std::string& directory,
    std::reference_wrapper<std::vector<std::string>> _sounds,
    std::reference_wrapper<std::vector<uint32_t>> _delays)
{
    auto& sounds = _sounds.get();
    auto& delays = _delays.get();

    if (directory.empty())
        return false;

    struct stat st;
    if (stat(directory.c_str(), &st))
    {
        std::cout << "Directory " << directory << " not found." << std::endl;
        return false;
    }

    // TODO Read all file names to sounds

    delays = std::move(std::vector<uint32_t>(sounds.size(), 0));
    return true;
}

bool fillConfigOneFile(
    const std::string& sndFile,
    std::reference_wrapper<std::vector<std::string>> _sounds,
    std::reference_wrapper<std::vector<uint32_t>> _delays)
{
    auto& sounds = _sounds.get();
    auto& delays = _delays.get();

    if (sndFile.empty())
        return false;

    struct stat st;
    if (stat(sndFile.c_str(), &st))
    {
        std::cout << "Sound file " << sndFile << " not found." << std::endl;
        return false;
    }

    sounds.push_back(sndFile);
    delays.push_back(0);
    return true;
}

int main(int argc, char** argv)
{
    const Cli cli = [argc, &argv]() {
        try
        {
            return parseCommandLine(argc, argv);
        }
        catch (std::runtime_error const& e) 
        {
            std::cout << e.what() << std::endl;
            exit(0);
        }
    }();

    if (!cli.check())
    {
        std::cout << "Only one of all options must be set" << std::endl;
        return -1;
    }

    std::vector<std::string> sounds;     
    std::vector<uint32_t> startDelays;
    
    if (!parseConfigFile(cli.cfgFile, sounds, startDelays) &&
        !fillConfigFromDirectory(cli.directory, sounds, startDelays) &&
        !fillConfigOneFile(cli.sndFile, sounds, startDelays))
    {
        std::cout << "No one sound file was set." << std::endl;
        return -1;
    }

    if (sounds.size() != startDelays.size()) 
    {
        std::cout << "Start delays' array size is not equal sounds' array size" << std::endl;
        return -1;
    }

    std::cout << "sndfiles: " << std::endl;
    for (uint32_t i = 0; i < sounds.size(); i++) {
        std::cout << " - " << sounds[i] << " : " << startDelays[i] << std::endl;
    }

     audiomixer::AudioMixerApi sndMixer;

     int timer = 0;
     double lastTime = 0.;

     std::cout << std::fixed;

     do{

          for (uint32_t i=0; i < sounds.size(); ++i) 
          {
               if (timer == startDelays[i])
               {
                    std::cout << "start " << i << " sound" << std::endl;
                    sndMixer.PlaySound(sounds[i]);

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
