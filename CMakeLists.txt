# Minimum CMake version
cmake_minimum_required(VERSION 3.10)

# Project name
project(MyProject LANGUAGES CXX)

# Set the C++ standard to C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Use Clang as the compiler
set(CMAKE_CXX_COMPILER clang++)

# Enable warnings and debugging symbols
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wpedantic -g")

# Find GLM (installed by brew or manually)
find_package(glm REQUIRED)

# Manually include GLM directory if not found automatically
if(NOT TARGET glm)
    set(GLM_INCLUDE_DIR /opt/homebrew/opt/glm/include)
    include_directories(${GLM_INCLUDE_DIR})
endif()

# Include directories (for header files)
include_directories(include)

# Source files
file(GLOB SOURCES "src/*.cpp")

# Add the executable
add_executable(MyProject ${SOURCES})