#!/bin/bash

# Step 1: Remove the build directory if it exists
if [ -d "build" ]; then
    rm -r build
fi

# Step 2: Create a new build directory and navigate into it
mkdir build
cd build

# Step 3: Generate the build system using CMake with Ninja generator
# Adding -DCMAKE_CXX_FLAGS_RELEASE="-O3" to enable the optimization level
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS_RELEASE="-O3" ..

# Step 4: Build the project in Release mode
cmake --build . --config Release --parallel

# Step 5: Remove any previous image file
rm ../image.ppm

# Step 6: Run the project and output to the image file
./MyProject >> ../image.ppm

# Step 7: Open the resulting image file using macOS Preview
open ../image.ppm