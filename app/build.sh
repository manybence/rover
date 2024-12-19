#!/bin/bash

# Automated script to compile and build the C code. This version includes the OpenCV libraries for automatic vessel detection.

# Build step
echo "Building..."
g++ -O3 -fopenmp -I/usr/include/opencv4 -Iiir -Wall -o "main" "main.cpp" -lpigpio -lrt -lpthread -Wno-psabi -lm -lopencv_core -lopencv_imgproc -lopencv_imgcodecs -lopencv_highgui -lopencv_videoio -lfftw3 -lfftw3_threads -std=c++17
if [ $? -ne 0 ]; then
    echo "Build failed."
    exit 1
fi

echo "Build successful. Executable created as 'main'."
