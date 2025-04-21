cmake_minimum_required(VERSION 3.10)
project(TerminalGame)

# Specify C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Tell CMake to use vcpkg for finding packages
if(DEFINED ENV{VCPKG_ROOT})
    message(STATUS "VCPKG_ROOT is set to $ENV{VCPKG_ROOT}")
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "Vcpkg toolchain file")
endif()

# If using vcpkg, you need to install PDCurses with:
# vcpkg install pdcurses

# Find PDCurses or ncurses based on platform
if(WIN32)
    # For Windows, find PDCurses
    find_path(PDCURSES_INCLUDE_DIR NAMES curses.h REQUIRED)
    find_library(PDCURSES_LIBRARY NAMES pdcurses REQUIRED)
    
    # Display what was found for debugging
    message(STATUS "Found PDCurses include: ${PDCURSES_INCLUDE_DIR}")
    message(STATUS "Found PDCurses library: ${PDCURSES_LIBRARY}")
    
    # Set variables for use later
    set(CURSES_INCLUDE_DIR ${PDCURSES_INCLUDE_DIR})
    set(CURSES_LIBRARIES ${PDCURSES_LIBRARY})
else()
    # For Unix-like systems, find ncurses
    find_package(Curses REQUIRED)
endif()

# Add include directories
include_directories(${CURSES_INCLUDE_DIR})

# Add the executable
add_executable(terminalgame terminalgame.cpp)

# Link libraries
target_link_libraries(terminalgame ${CURSES_LIBRARIES})
