cmake_minimum_required(VERSION 3.14)

find_package(SDL2 REQUIRED)

set(TARGET_NAME soundmixer_demo)
set(SRC_DIR ".")
set(LOCAL_BIN_OUT_DIR "../${BIN_OUT_DIR}")
project(${TARGET_NAME} 
    VERSION 0.0.1 
    LANGUAGES CXX C
    DESCRIPTION "Demonstration for sound mixer library")

include_directories(. .. ../third_party/rapidjson/include ../soundmixer_lib)
link_directories(../lib)

add_executable(${TARGET_NAME} 
    ${SRC_DIR}/cl_parser.cpp 
    ${SRC_DIR}/mixer_demo.cpp
    ${SRC_DIR}/platform_sdl.cpp
    ${SRC_DIR}/sdl/src/audio.c)

target_compile_features(${TARGET_NAME} PRIVATE cxx_std_17)
set_target_properties(${TARGET_NAME}  
    PROPERTIES 
        RUNTIME_OUTPUT_DIRECTORY ${LOCAL_BIN_OUT_DIR})

target_link_libraries(${TARGET_NAME} soundmixer SDL2)
