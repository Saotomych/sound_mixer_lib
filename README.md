# Sound Mixer Library

### Introduction

The Sound Mixer Library mixes unlimited amount of sounds taken from wav sound streams
into a stream of the same wav format.

The Sound Mixer Library is recommended to use for all the systems that do not include either software or hardware sound mixer logic. For instance, it can be any
of embedded or IoT systems with data storage for wav sound files. The system
must have performance to provide data streams of the all mixed sound files into
a sound player.

### Building and installation

The project uses building system CMake, so for demo building is enough to run:
 * "cmake ." to prepare makefiles
 * "make" to build

Using CMake environment variables is needed to build the library for your platform.

Building system also uses its own additional CMake options for settings:
 * SOUNDMIXER_BUILD_SHARED_LIB - Build a shared library (.dll/.so) instead of 
static one (.lib/.a). Default: OFF.
 * SOUNDMIXER_BUILD_DEBUG_LIB - Build debug version of the library. Default: OFF.
 * SOUNDMIXER_BUILD_TEST_LIB - Build unit tests. Default: ON.
 * SOUNDMIXER_BUILD_DEMO_LIB - Build a demo application. Default: ON.
 * SOUNDMIXER_BUILD_DOCS_LIB - Build documentation. Default: ON.
 * SOUNDMIXER_MULTITHREAD_LIB - Build the Library with multithreading support. Default: ON.

CMake provides tool for installing the library to your system automatically with 
a command: make install.
By default, the library builds to ./lib. And demo binary file builds to ./bin.

### Integration to your project
 
For working with your project environment you need to implement set of 
functions supporting your platform dependencies. 
See the examples of the functions implementations in the source of the demo application.
Descriptions of all of the functions are in the include/platform_deps.h file.

### Using a demo application
 
The demo can play one sound file or all files from the one directory or
files according to the config file. Config file also allows setting the start time for the next files regarding the start of the first file playback. Start time of the first file is always null.

Config file has JSON format and has 3 mandatory fields:
 * "directory" is a prefix for all filenames in configuration
 * "files" is a list of the sound filenames
 * "delays" is a list of start points in 0,1 second. First point must be 0.

To run the demo you need to prepare your own sound files or download an example from
https://drive.google.com/file/d/1boRMdjjFdPF1prEnnw9NRq3quInD_VF_/view?usp=sharing
There are wav files and the config file for them.
Extract files from archive to the directory ./bin where the audiomixer_demo file is and run the example: ./audiomixer_demo -c cfg/audio_nature.json.cfg.

You can also prepare your own sound files. For instance, using the ffmpeg tool helps to do it.
Example: ffmpeg -i in.mp3 -acodec pcm_s16le -ac 1 -ar 11025 out.wav

Library source code is written with C++11 because family of embedded compilers
are not able to support higher standards. So you can use your compiler
for the library building. Also, for simple projects you can take sound mixing code 
directly from the library sources.

Mixing expression is implemented in a function SoundMixer::GetMixedBuffer.
You can replace the call to the reading from the sound stream function (reader.second.NextDataBlock) to your own data reading function and gain a stand-alone implementation.

### How it works

Sound mixing is not such a simple task. The solution is based on the theory 
of relativity. We widen it by involving optical processes.
Velocity addition rule is not canceled here, but is only clarified for high 
speeds. The Lorentz transformation helps us with it.
The main expression is: vrel = (v1+v2)/(1+(v1*v2)/c^2)

We can see that in the boundary case when the Lorentz transformation converts
into Galileo's transformation the special theory of relativity becomes similar to Newton's mechanics on speeds close to the light speed.

This explains how both theories (relativity theory and Newton’s mechanic) can be combined to gain the more exact definition of Newton’s mechanic, which allows implementing it to the sound waves effects.

In the source code below the light speed is changed to the wav stream maximum sample value and the speed changes from other sound streams are added to the actual sample values.

    int light speed = 32768*32768; // max short ^ 2 for 16-bit sound
    for (int i = 0; i < bufflen; ++i) {
        float a = buf1[i];
        float b = buf2[i];
        float rel_samp = (a + b) / (1 + (a * b) / light speed);
        out_buf[i] = rel_samp;
    }

### License information

The library and demo software licensed under Creative Commons Zero v 1.0.

SDL2.0 API used in demo software. SDL2.0 API sources licensed under the Apache License, Version 2.0.

