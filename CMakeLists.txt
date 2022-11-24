cmake_minimum_required(VERSION 3.9)

set(TARGET_NAME audiomixer_demo)
set(SRC_DIR "demo")
set(OUT_DIR "bin")
project(${TARGET_NAME} 
    VERSION 0.0.1 
    LANGUAGES CXX 
    DESCRIPTION "Demonstration for audio mixer library")

add_subdirectory(audiomixer_lib)

include_directories(. ./third_party/include/rapidjson/include ./audiomixer_lib)
link_directories(lib)

add_executable(${TARGET_NAME} 
    ${SRC_DIR}/cl_parser.cpp 
    ${SRC_DIR}/mixer_demo.cpp)

target_compile_features(${TARGET_NAME} PRIVATE cxx_std_17)
set_target_properties(${TARGET_NAME}  
    PROPERTIES 
        RUNTIME_OUTPUT_DIRECTORY ${OUT_DIR})

target_link_libraries(${TARGET_NAME} audiomixer SDL2)
