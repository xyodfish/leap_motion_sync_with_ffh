cmake_minimum_required(VERSION 3.16.0)
project(
    leap-motion-ffh
    VERSION 0.0.1
    LANGUAGES C CXX)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(NOT MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)
set(CMAKE_BUILD_TYPE Debug)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
    message("Setting default build type to Release")
else()
    message("build type is setting to ${CMAKE_BUILD_TYPE}")
endif()

if (UNIX)    
    find_package(Threads REQUIRED)    
endif (UNIX)

set(INC_DIR ${PROJECT_SOURCE_DIR}/include)
set(SRC_DIR ${PROJECT_SOURCE_DIR}/src)

if(WIN32)
    set(LIB_DIR ${PROJECT_SOURCE_DIR}/lib/win)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

    if(MSVC)
        if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
            set(LIB_THIRD_PARTY_DIR ${LIB_DIR}/third_party/msvc/debug)
        else()
            set(LIB_THIRD_PARTY_DIR ${LIB_DIR}/third_party/msvc/release)
        endif()
    else()
        set(LIB_THIRD_PARTY_DIR ${LIB_DIR}/third_party/mingw)
    endif()
else()
    set(LIB_DIR ${PROJECT_SOURCE_DIR}/lib/linux)
    set(LIB_THIRD_PARTY_DIR ${LIB_DIR}/third_party)
endif()

set(TASK ${PROJECT_SOURCE_DIR}/task)
set(COMMON ${SRC_DIR}/common)
set(UDP ${SRC_DIR}/udp)
set(FFH ${SRC_DIR}/FFH)


set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)


add_subdirectory(${SRC_DIR}/udp)
add_subdirectory(${SRC_DIR}/FFH)
add_subdirectory(${SRC_DIR}/task)
add_subdirectory(${SRC_DIR}/OfficialWrapper)

