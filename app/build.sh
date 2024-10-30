#!/bin/bash

# Automated script to compile and build the C code.

# Compilation step
#echo "Compiling..."
#g++ -O3 -fopenmp -lwiringPi -I/usr/include/opencv4 -Iiir -Wall -c "main.cpp" -Wno-psabi -lfftw3 -lm -std=c++17
#if [ $? -ne 0 ]; then
#    echo "Compilation failed."
#    exit 1
#fi

# Build step
echo "Building..."
g++ -O3 -fopenmp -I/usr/include/opencv4 -Iiir -Wall -o "main" "main.cpp" -lpigpio -lrt -lpthread -Wno-psabi -lm -lopencv_core -lopencv_imgproc -lopencv_imgcodecs -lopencv_highgui -lopencv_videoio -lfftw3 -lfftw3_threads -std=c++17
if [ $? -ne 0 ]; then
    echo "Build failed."
    exit 1
fi

# Linting step
#echo "Linting..."
#cppcheck --language=c++ --enable=warning,style --template=gcc "main.cpp"

echo "Build successful. Executable created as 'main'."
