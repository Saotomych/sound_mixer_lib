cmake_minimum_required(VERSION 3.9)

set(TARGET_NAME audiomixer)

project(${TARGET_NAME} VERSION 0.0.1 DESCRIPTION "Embedded audio mixer")

add_library(${TARGET_NAME} SHARED
        audiomixer/src/audioapi.cpp
        audiomixer/src/audiomixer.cpp
        audiomixer/src/audioreader.cpp
        audio_platform/src/platform_sdl.cpp
        )

target_compile_features(${TARGET_NAME} PRIVATE cxx_std_11)
target_include_directories(${TARGET_NAME} PRIVATE .)

set_target_properties(${TARGET_NAME} PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(${TARGET_NAME} PROPERTIES SOVERSION 1)
set_target_properties(${TARGET_NAME} PROPERTIES PUBLIC_HEADER audiomixer/audiomixer_api.h)

install(TARGETS ${TARGET_NAME}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
