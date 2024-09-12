# Minimum CMake version
cmake_minimum_required(VERSION 3.10)

# Project name
project(main LANGUAGES CXX)

# Set the C++ standard to C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Use Clang as the compiler
set(CMAKE_CXX_COMPILER clang++)

# Enable warnings and debugging symbols
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wpedantic -g")

# Set the build type to Debug by default
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

# Include directories (for header files)
include_directories(include)

# Source files
file(GLOB SOURCES "src/*.cpp")

# Add the executable
add_executable(main ${SOURCES})