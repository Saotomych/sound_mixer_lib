cmake_minimum_required(VERSION 3.14)

include(CMakePackageConfigHelpers)

set(TARGET_NAME soundmixer)
set(LOCAL_LIB_OUT_DIR "../${LIB_OUT_DIR}")
set(LOCAL_BIN_OUT_DIR "../${BIN_OUT_DIR}")

project(${TARGET_NAME} VERSION 0.0.1 DESCRIPTION "Sound mixer library")

if(DEFINED SOUNDMIXER_BUILD_SHARED_LIB)
    set(BUILD_SHARED_LIBS ${SOUNDMIXER_BUILD_SHARED_LIB})
endif()

add_library(${TARGET_NAME}) # initialized below
add_library(${TARGET_NAME}::${TARGET_NAME} ALIAS ${TARGET_NAME})

target_compile_features(${TARGET_NAME} PRIVATE cxx_std_11)
target_include_directories(${TARGET_NAME} PRIVATE .)

set(public_headers
    include/soundmixer_api.h
    include/platform_deps.h)

set(sources
    ${public_headers}
    soundmixer/src/soundmixer_api.cpp
    soundmixer/src/soundmixer.cpp
    soundmixer/src/soundreader.cpp)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${sources})

target_sources(${TARGET_NAME} PRIVATE ${sources})

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
        LIBRARY_OUTPUT_DIRECTORY ${LOCAL_LIB_OUT_DIR}
        ARCHIVE_OUTPUT_DIRECTORY ${LOCAL_LIB_OUT_DIR})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -Werror")
if (DEFINED SOUNDMIXER_BUILD_DEBUG_LIB AND SOUNDMIXER_BUILD_DEBUG_LIB MATCHES ON)
    set(CMAKE_CXX_FLAGS "-gdwarf-3 -O0 ${CMAKE_CXX_FLAGS}")
else()
    set(CMAKE_CXX_FLAGS "-O3 ${CMAKE_CXX_FLAGS}")
endif()

if (DEFINED SOUNDMIXER_BUILD_TEST_LIB AND SOUNDMIXER_BUILD_TEST_LIB MATCHES ON)
    add_subdirectory(test)
endif()

if (DEFINED SOUNDMIXER_BUILD_DOCS_LIB AND SOUNDMIXER_BUILD_DOCS_LIB MATCHES ON)
    find_package(Doxygen)
    if (DOXYGEN_FOUND)
        set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/../docs/Doxyfile.in)
        set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

        configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
        message("Doxygen build started")

        # note the option ALL which allows to build the docs together with the application
        add_custom_target( doc_doxygen ALL
            COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generating API documentation with Doxygen"
            VERBATIM )
    else()
      message("Doxygen need to be installed to generate the doxygen documentation")
    endif()
endif()
