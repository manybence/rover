#ifndef motors_h
#define motors_h

#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <linux/serial.h>
#include <fcntl.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>
#include <termios.h> 
#include <algorithm>
#include "serial_comm.h"
#include "base64.h"

std::vector<int> timeSeries;
std::vector<int> positionSeries;

bool IsMotorZeroed() {
    pack_command('o', 0);   // Send command to read log
    
    // Read log bytes
    std::string receivedData = readResponse();
    return stringToBool(receivedData);
}

void MoveMotorToPosition(float xpos) {

    pack_command('m', xpos);
    std::cout << readResponse() << std::endl;
}

void MotorSpeed(float xspeed) {
    pack_command('s', xspeed);
    std::cout << readResponse() << std::endl;
}

void MoveMotor500um(int sleepdelay) {
    pack_command('+', 0.5);
    usleep(sleepdelay); // wait for motor to settle on position
}

void InitMotorPosition() {

    std::cout << "Resetting motor" << std::endl;
    
    // Reconfiguration
    pack_command('c' , 0);
    std::cout << readResponse() << std::endl;
    
    // Reset to zero position
    pack_command('z', 0);
    std::cout << readResponse() << std::endl;
}

float interpolatePosition(float time) {
    
    // Ensure time is within the range
    if (time <= timeSeries.front()) return static_cast<float>(positionSeries.front());
    if (time >= timeSeries.back()) return static_cast<float>(positionSeries.back());

    // Find the appropriate interval for interpolation
    for (size_t i = 1; i < timeSeries.size(); ++i) {
        if (time < static_cast<float>(timeSeries[i])) {
            float t1 = static_cast<float>(timeSeries[i - 1]);
            float t2 = static_cast<float>(timeSeries[i]);
            float x1 = static_cast<float>(positionSeries[i - 1]);
            float x2 = static_cast<float>(positionSeries[i]);

            // Perform linear interpolation
            return x1 + (time - t1) * (x2 - x1) / (t2 - t1);
        }
    }
    return static_cast<float>(positionSeries.back());
};

void decodeLog(const std::string& encodedLog) {
    
    std::cout << "Decoding log\n" << std::endl;
    
    int X0, X1;
    std::string deltaEncoded;
    std::string encodedPair;

    // Initialize variables
    int deltaTime = 0;
    int T = 0;

    // Parse the header [X0,X1]
    int startIndex = encodedLog.find('[') + 1;
    int commaIndex = encodedLog.find(',', startIndex);
    int endIndex = encodedLog.find(']', commaIndex);

    // Search for log variables
    try {
        X0 = std::stoi(encodedLog.substr(startIndex, commaIndex - startIndex));
        X1 = std::stoi(encodedLog.substr(commaIndex + 1, endIndex - commaIndex - 1));
        deltaEncoded = encodedLog.substr(endIndex + 1, encodedLog.find('#') - endIndex - 1);
    } catch (const std::invalid_argument& e) {
        std::cout << "Error: Invalid number format in the log header." << std::endl;
        return;
    } catch (const std::out_of_range& e) {
        std::cout << "Error: Number out of range in the log header." << std::endl;
        return;
    }


	// Populate positionSeries from X0 to X1
    positionSeries.push_back(X0);
    if (X0 <= X1) {
        for (int pos = X0; pos <= X1; ++pos) {
            positionSeries.push_back(pos);
        }
    } else {
        for (int pos = X0; pos >= X1; --pos) {
            positionSeries.push_back(pos);
        }
    }

    // Populate timeSeries
    timeSeries.push_back(0);
    for (size_t i = 0; i < deltaEncoded.length(); i += 2) {
        encodedPair = deltaEncoded.substr(i, 2);
        deltaTime = Base64::b64ConvertString(encodedPair);
        if (deltaTime > 4096) deltaTime = 4096; // Safety threshold

        // Reconstruct the full time series
        T += deltaTime;
        timeSeries.push_back(T);
    }
    
    std::cout << "Starting pos: " << X0 << std::endl;
    std::cout << "Ending pos: " << X1 << std::endl;
    std::cout << "Time elapsed: " << T << std::endl;
} 

void GetLog() {
    
    std::cout << "Reading Log" << std::endl;
    tcflush(fd, TCIOFLUSH); // Discard both input and output data
    pack_command('l', 0);   // Send command to read log
    
    // Read log bytes
    std::string receivedData = readResponse();
    decodeLog(receivedData);
}

void NeedleMotorPosition(float p) {
    
    char pos = '0';

    if (p < 0.5)                 pos = '0';
    if ((p <= 0.5) && (p < 1.5)) pos = '1';
    if ((p <= 1.5) && (p < 2.5)) pos = '2';
    if ((p <= 2.5) && (p < 3.5)) pos = '3';
    if ((p <= 3.5) && (p < 4.5)) pos = '4';

    pack_command('p', pos);
    usleep(10000);
}

float ReadMotorPosition() {

    pack_command('g', 0);
    std::string position_string = readResponse();

    float position = 0.0;

    // Extract position
    try {
        position = stringToFloat(trim(position_string));
    } 
    catch (const std::invalid_argument& e) {
        std::cout << "Error: Invalid format in the response." << std::endl;
    }

    return position;
}

#endif
