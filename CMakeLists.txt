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

add_subdirectory(soundmixer_lib)

find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
    option(GIT_SUBMODULE "Check submodules during build" ON)
    if(GIT_SUBMODULE)
        message(STATUS "Submodule update")
        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                        RESULT_VARIABLE GIT_SUBMOD_RESULT)
        if(NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(FATAL_ERROR "git submodule update --init --recursive failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
        endif()
    endif()
endif()


if(DEFINED SOUNDMIXER_BUILD_DEMO_LIB AND SOUNDMIXER_BUILD_DEMO_LIB MATCHES ON)
    if(NOT EXISTS "${PROJECT_SOURCE_DIR}/third_party/rapidjson/CMakeLists.txt")
        message(FATAL_ERROR "The submodule 'rapidjson' was not downloaded!")
    else()
        add_subdirectory(demo)
    endif()
endif()

if(DEFINED SOUNDMIXER_BUILD_TEST_LIB AND SOUNDMIXER_BUILD_TEST_LIB MATCHES ON)
    if(NOT EXISTS "${PROJECT_SOURCE_DIR}/third_party/gtest/CMakeLists.txt")
        message(FATAL_ERROR "The submodule 'gtest' was not downloaded!")
    else()
        add_subdirectory(third_party/gtest)
        add_test(NAME MixTest COMMAND "${BIN_OUT_DIR}/soundmixer_test")
    endif()
endif()
