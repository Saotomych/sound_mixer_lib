cmake_minimum_required(VERSION 3.14)

set(TARGET_NAME soundmixer_test)

set(SRC_DIR ".")
set(LOCAL_BIN_OUT_DIR "../../${BIN_OUT_DIR}")
set(LOCAL_LIB_OUT_DIR "../../${LIB_OUT_DIR}")
project(${TARGET_NAME} 
    VERSION 0.0.1 
    LANGUAGES CXX
    DESCRIPTION "Tests for audio mixer library")

include_directories(.. ../../third_party/gtest/googletest/include)
link_directories(LOCAL_LIB_OUT_DIR)

add_executable(${TARGET_NAME} ${SRC_DIR}/test.cpp)

target_compile_features(${TARGET_NAME} PRIVATE cxx_std_17)
set_target_properties(${TARGET_NAME}  
    PROPERTIES 
        RUNTIME_OUTPUT_DIRECTORY ${LOCAL_BIN_OUT_DIR})

target_link_libraries(${TARGET_NAME} gtest gtest_main soundmixer)
