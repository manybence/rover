#!/bin/bash

# Automated script to compile and build the C code. This version excludes the OpenCV libraries.

# Build step
echo "Building..."
g++ -O3 -fopenmp -Iiir -Wall -o "main" "main.cpp" "lib/gpio_handler.cpp" -lgpiod -lrt -lpthread -Wno-psabi -lm -lfftw3 -lfftw3_threads -lcrypto -std=c++17 -lboost_filesystem -lboost_system
if [ $? -ne 0 ]; then
    echo "Build failed."
    exit 1
fi

echo "Build successful. Executable created as 'main'."
