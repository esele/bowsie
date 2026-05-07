cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

include(FetchContent)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS ON)

option(ASAR_DYNAMIC_LINK "Whether to link Asar dynamically." ON)

if(ASAR_DYNAMIC_LINK)
    set(ASAR_TARGET "asar")
    set(ASAR_GEN_DLL ON)
    set(ASAR_GEN_LIB OFF)
else()
    set(ASAR_TARGET "asar-static")
    set(ASAR_GEN_DLL OFF)
    set(ASAR_GEN_LIB ON)
endif()
set(ASAR_GEN_DLL_TEST OFF)

FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt
    GIT_TAG        40626af88bd7df9a5fb80be7b25ac85b122d6c21
)
FetchContent_MakeAvailable(fmt)

FetchContent_Declare(
    asar
    GIT_REPOSITORY https://github.com/RPGHacker/asar
    GIT_TAG v1.91
    SOURCE_SUBDIR src
)
FetchContent_MakeAvailable(asar)

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
FetchContent_MakeAvailable(json)

set_target_properties(${ASAR_TARGET} PROPERTIES CXX_STANDARD 14)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    if(ASAR_DYNAMIC_LINK)
        set_target_properties(${ASAR_TARGET} PROPERTIES COMPILE_OPTIONS "-w")
    else()
        set_target_properties(${ASAR_TARGET} PROPERTIES COMPILE_OPTIONS "-w;-fPIC")
    endif()
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set_target_properties(${ASAR_TARGET} PROPERTIES COMPILE_OPTIONS "/w")
else ()
    message(FATAL_ERROR "Unrecognized compiler")
endif()

