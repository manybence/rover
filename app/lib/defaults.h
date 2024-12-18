#ifndef defaults_h
#define defaults_h

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include "parameters.h"
#include "json.hpp"

std::string default_param_path = "rover/app/lib_gui/default_parameters.json";

std::unordered_map<std::string, std::string> loadDefaultParams(const std::string& filename) {
    std::unordered_map<std::string, std::string> params;
    
    // Open the JSON file
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Unable to open JSON file: " + filename);
    }
    
    // Parse the JSON file
    nlohmann::json jsonData;
    file >> jsonData;

    // Iterate over the JSON data and populate the map
    for (auto& [key, value] : jsonData.items()) {
        params[key] = value.dump();  // Store values as strings
    }

    return params; // Return the populated map
}

std::unordered_map<std::string, std::string> parameters;

// The amount the DAC controlling the VGA get dec each adjustment to compensate for tissue attenuation
#define TISSUE10M 104;
#define TISSUE5M   54;
#define WATER10M    0;


// Initialize variables
int lines       =   100;
int offsetmin   =     5;
int offsetmax   =    42;
int angle       =    45;
int manualgain  =   400;
float xposmin   =   5.0; //mm
float xposmax   = 50.0; //1/10 mm
float xspeed     = 8.0;   // 8 mm/s
float xstep = 1.0; //mm
int filsel = 0;
int scanning_time = 3000;   // M-mode scanning (ms)
unsigned char txpat = TXPAT2;
int gainrate    = TISSUE10M;
const int ARRAY_SIZE = 7168;
bool IsDopplerMode = false;
int h;
bool configured = false;
int pos_final = 0;

// The following tables is for 60 deg doppler angle 10 MHz probe (SensorScan) STRATEG is a strategy for start serching the most likely place of the FA.
#define STGHIGH   32 * 4
#define UNTILHILO     15

//A-MODE 90 deg., DMax=63.7 mm, fsam = 60 MHz, BufferLength = 4004
#define A_MODE_BUFLEN 4004
#define A_MODE_READOUT_OFFSET 960

//Test offset (for phantom - probe near target. Note Offset 0 is resulting in a wrong capture)
int STRATEG[STGHIGH] = {   0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
                          32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
                          64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95,
                          96, 97, 98, 99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127
                       };
int dac_val[STGHIGH] = {1023, 1023, 1023, 1023, 1023, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900,
                         900,  900,  900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900,
                         900,  900,  900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900,
                         900,  900,  900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900
                       };
int hiloval[STGHIGH] = {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                          0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                          0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                          0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
                       };
int DOPP_OF[STGHIGH] = {   1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32,
                          33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64,
                          65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96,
                          97, 98, 99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,128 };



#endif
