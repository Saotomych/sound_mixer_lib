cmake_minimum_required(VERSION 3.14)

option(SOUNDMIXER_BUILD_SHARED_LIB "Build shared libraries (.dll/.so) instead of static ones (.lib/.a)" OFF)
option(SOUNDMIXER_BUILD_DEBUG_LIB "Build debug version of the library" OFF)
option(SOUNDMIXER_BUILD_TESTS "Build and run unit tests" ON)

set(SOUNDMIXER_BUILD_SHARED_LIB OFF)
set(SOUNDMIXER_BUILD_DEBUG_LIB, OFF)
set(SOUNDMIXER_BUILD_TESTS, ON)

project(soundmixer
    VERSION 0.0.1 
    LANGUAGES CXX 
    DESCRIPTION "Demonstration for audio mixer library")

add_subdirectory(audiomixer_lib)
add_subdirectory(demo)
