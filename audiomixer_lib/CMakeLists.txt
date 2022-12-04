cmake_minimum_required(VERSION 3.14)

include(CMakePackageConfigHelpers)

set(TARGET_NAME soundmixer)
set(OUT_DIR "../lib")

project(${TARGET_NAME} VERSION 0.0.1 DESCRIPTION "Embedded audio mixer")

#if(DEFINED SOUNDMIXER_BUILD_SHARED_LIB)
    set(BUILD_SHARED_LIBS ${SOUNDMIXER_BUILD_SHARED_LIB})
#endif()

add_library(${TARGET_NAME}) # initialized below
add_library(${TARGET_NAME}::${TARGET_NAME} ALIAS ${TARGET_NAME})

option(AUDIOMIXER_BUILD_TESTS "Build audiomixer tests" OFF)

target_compile_features(${TARGET_NAME} PRIVATE cxx_std_11)
target_include_directories(${TARGET_NAME} PRIVATE .)

set(public_headers
    include/audiomixer_api.h)

set(sources
    ${public_headers}
    audiomixer/src/audioapi.cpp
    audiomixer/src/audiomixer.cpp
    audiomixer/src/audioreader.cpp
    audio_platform/src/platform_sdl.cpp
    sdl/src/audio.c)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${sources})

target_sources(${TARGET_NAME} PRIVATE ${sources})

target_compile_definitions(${TARGET_NAME}
    PUBLIC
        "$<$<NOT:$<BOOL:${BUILD_SHARED_LIBS}>>:SOUNDMIXER_STATIC_DEFINE")

target_include_directories(${TARGET_NAME}
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src")

set_target_properties(${TARGET_NAME} 
    PROPERTIES 
        VERSION ${PROJECT_VERSION}
        SOVERSION 1
        PUBLIC_HEADER "${public_headers}"
        LIBRARY_OUTPUT_DIRECTORY ${OUT_DIR}
        ARCHIVE_OUTPUT_DIRECTORY ${OUT_DIR})

if (BUILD_DEBUG_LIB EQUAL 1)
    add_compile_options(-gdwarf-3 -O0 -Wall -Wextra -pedantic -Werror)
else()
    add_compile_options(-O3 -Wall -Wextra -pedantic -Werror)
endif()
