cmake_minimum_required(VERSION 3.14)

option(SOUNDMIXER_BUILD_SHARED_LIB "Build shared libraries (.dll/.so) instead of static ones (.lib/.a)" OFF)
option(SOUNDMIXER_BUILD_DEBUG_LIB "Build debug version of the library" OFF)
option(SOUNDMIXER_BUILD_TEST_LIB "Build unit tests" ON)
option(SOUNDMIXER_BUILD_DEMO_LIB "Build demo application" ON)
option(SOUNDMIXER_BUILD_DOCS_LIB "Build documentation" ON)

set(SOUNDMIXER_BUILD_SHARED_LIB OFF)
set(SOUNDMIXER_BUILD_DEBUG_LIB ON)
set(SOUNDMIXER_BUILD_TEST_LIB ON)
set(SOUNDMIXER_BUILD_DEMO_LIB ON)
set(SOUNDMIXER_BUILD_DOCS_LIB ON)
set(LIB_OUT_DIR "./lib")
set(BIN_OUT_DIR "./bin")

#set(CMAKE_VERBOSE_MAKEFILE ON)

project(soundmixer VERSION 0.0.1 DESCRIPTION "Sound mixer library and environment")

include(CTest)

add_subdirectory(audiomixer_lib)

if(DEFINED SOUNDMIXER_BUILD_DEMO_LIB AND SOUNDMIXER_BUILD_DEMO_LIB MATCHES ON)
    add_subdirectory(demo)
endif()

if(DEFINED SOUNDMIXER_BUILD_TEST_LIB AND SOUNDMIXER_BUILD_TEST_LIB MATCHES ON)
    add_subdirectory(third_party/gtest)
    add_test(NAME MixTest COMMAND "${BIN_OUT_DIR}/soundmixer_test")
endif()
