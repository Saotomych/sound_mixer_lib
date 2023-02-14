/*! \mainpage notitle
 *
 * \section intro_sec Introduction
 *
 * The sound mixer library mixes unlimited amount of sounds from wav sound files
 * into the same wav format.
 *
 * The sound mixer library is recommended to use for all systems not including
 * either software or hardware sound mixer logic. As instance, it can be any
 * of embedded or IoT system with data storage for wav sound files. The system
 * must have performance to provide data streams of the all mixed sound files to
 * a sound player.
 *
 * \section install_sec Building and installation
 *
 * The project uses build system cmake so for demo building is enough to run:
 *  * "cmake ." to prepare makefiles
 *  * "make" to build
 *
 * For any cross platform applyings is needed to use cmake environment variables
 * to build library for your platform.
 *
 * Build system uses also its own additional cmake options for settings:
 * - SOUNDMIXER_BUILD_SHARED_LIB - Build shared libraries (.dll/.so) instead of 
 * static ones (.lib/.a). Default: OFF.
 * - SOUNDMIXER_BUILD_DEBUG_LIB - Build debug version of the library. 
 * Default: OFF.
 * - SOUNDMIXER_BUILD_TEST_LIB - Build unit tests. Default: ON.
 * - SOUNDMIXER_BUILD_DEMO_LIB - Build demo application. Default: ON.
 * - SOUNDMIXER_BUILD_DOCS_LIB - Build documentation. Default: ON.
 * - SOUNDMIXER_MULTITHREAD_LIB - Build the Library with multithreading support.
 * Default: ON.
 * CMake provides tool to install library to your system automatically with 
 * a command: make install.
 * As default, library builds to ./lib. Meanwhile, demo binary builds to ./bin.
 *
 * \section integration_sec Integration to your project
 * 
 * For work with your project environment, you need to implement set of 
 * functions supporting your platform dependencies. 
 * Examples of their implementations you can see in the demo project.
 * Descriptions of all of the functions are in include/platform_deps.h file.
 *
 * \section using_sec Using
 * 
 * The demo can play one sound file, all files from the one directory and
 * files according to config file. Demo can take start points for the certain
 * file from config file only.
 *
 * Config file has JSON format and has 3 mandatory fields:
 * - "directory" is a prefix for all filenames in config
 * - "files" is a list of the sound filenames
 * - "delays" is a list of start points in 0,1 seconds. Fist point mast be 0.
 *
 * To run the demo you need to prepare your own sound files or download example from
 * https://drive.google.com/file/d/1boRMdjjFdPF1prEnnw9NRq3quInD_VF_/view?usp=sharing
 * There are wav files and config file for them.
 * Extract files form archive to the directory bin where is audiomixer_demo file.
 * and run example: ./audiomixer_demo -c cfg/audio_nature.json.cfg
 *
 * You can also prepare your own sound files. In instance, it helps to do ffmpeg tool.
 * Example: ffmpeg -i in.mp3 -acodec pcm_s16le -ac 1 -ar 11025 out.wav
 *
 * Library source code is written with C++11 because family of embedded compilers
 * are not be able to support more higher standards. So we can use your compiler
 * for static library building. Also for simple projects you can take mixing code 
 * directly from the sources. 
 * Mixing expression is implemented in SoundMixer::GetMixedBuffer.
 * You can change reading expression reader.second.NextDataBlock to your own
 * file reading function and gain stand-alone implementation.
 *
 * \section work_sec How it works
 *
 * Sound mixing is not a such simple task. The solution is based on the theory 
 * of relativity. It's used here more wide involving optical process.
 * The rule for adding speeds is not canceled here, but only clarified for high 
 * speeds. The Lorentz transformation helps us with it. 
 * The main expression is: vrel = (v1+v2)/(1+(v1*v2)/c^2)
 * We can see that in the border case when the Lorentz transformation converts
 * to the Galileo's transformation the special theory of relativity is the same
 * as the Newton's mechanics with more less speeds than light speed.
 * It explains how to mix both theories: the first theory is more precise definition
 * of the second theory.
 * 
 * In a source code, the light speed changes to maximum sample value and adding 
 * speeds changes to the actual sample values
 *
 * @code
 *
 * int light speed = 32768*32768; // max short ^ 2 for 16-bit sound
 * for (int i = 0; i < bufflen; ++i) {
 *     float a = buf1[i];
 *     float b = buf2[i];
 *     float rel_samp = (a + b) / (1 + (a * b) / light speed);
 *     out_buf[i] = rel_samp;
 * }
 *
 * @endcode  
 *
 * \section license_sec License information
 *
 * Library and demo software licensed under Creative Commons Zero v 1.0.
 *
 * SDL2.0 API used in demo software and licensed under the Apache License, Version 2.0.
 *
 */
