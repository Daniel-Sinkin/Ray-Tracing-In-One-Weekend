#!/bin/bash

# Remove the build directory if it exists
if [ -d "build" ]; then
    rm -r build
fi

# Create a new build directory
mkdir build
cd build

# Run CMake with the Ninja generator
cmake -G Ninja ..

# Build the project using all available cores
cmake --build . --parallel