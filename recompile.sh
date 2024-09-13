#!/bin/bash

# Step 1: Remove the build directory if it exists
if [ -d "build" ]; then
    rm -r build
fi

# Step 2: Create a new build directory and navigate into it
mkdir build
cd build

# Step 3: Generate the build system using CMake with Ninja generator
cmake -G Ninja ..

# Step 4: Build the project in Release mode
cmake --build . --config Release --parallel

# Step 5:
rm ../image.ppm
./MyProject >> ../image.ppm

# Step 6: Open the resulting image file using macOS Preview
open ../image.ppm