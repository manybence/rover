#ifndef defaults_h
#define defaults_h

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include "parameters.h"
#include "json.hpp"
#include <boost/filesystem.hpp>

std::string default_param_path = "./lib_gui/default_parameters.json";

std::unordered_map<std::string, std::string> loadDefaultParams(const std::string& filename) {
    std::unordered_map<std::string, std::string> params;

    // Check if the file exists
    if (!boost::filesystem::exists(filename)) {
        throw std::runtime_error("File does not exist: " + filename);
    }
    
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
        if (value.is_string()) {
            // Store string as string (no quotes)
            params[key] = value.get<std::string>();
        } else {
            // Fall back to storing as a JSON string representation
            // only if it's not a plain string
            params[key] = value.dump();
        }
        //std::cout << "params[key]: [" << params[key] << "]" << std::endl;
    }

    return params; // Return the populated map
}

// Default values
const std::string XPOSMIN_DEF            =            "5.0";
const std::string XPOSMAX_DEF            =           "50.0";
const std::string XSPEED_DEF             =            "8.0";
const std::string ZPOSMIN_DEF            =            "0.0";
const std::string ZPOSMAX_DEF            =            "4.0";
const std::string A_MODE_OFFSETMIN_DEF   =              "0";
const std::string A_MODE_OFFSETMAX_DEF   =              "2";
const std::string A_MODE_AUTOGAIN_DEF    =           "true";
const std::string A_MODE_MANUALGAIN_DEF  =            "700";
const std::string A_MODE_GAINRATE_DEF    =             "90"; //105 For 10 MHz probe  //90 for 5 MHz probe
const std::string A_MODE_FILTERTYPE_DEF  =            "HPF";
const std::string M_MODE_SCANTIME_DEF    =        "3000000";    // 3000000 us = 3 s
const std::string DOPPLER_OFFSETMIN_DEF  =              "5";
const std::string DOPPLER_OFFSETMAX_DEF  =              "5";//50
const std::string DOPPLER_AUTOGAIN_DEF   =          "false";
const std::string DOPPLER_MANUALGAIN_DEF =           "1023";
const std::string DOPPLER_FILTERTYPE_DEF =            "HPF";
const std::string DOPPLER_SCANLINES_DEF  =              "4";
const std::string A_MODE_PORT_DEF        =             "X1";
const std::string DOPPLER_PORT_DEF       =             "X3";
const std::string DOPPLER_ANGLE_DEF      =              "0";
const std::string MODE_DEF               =         "DOPPLER"; //"DOPPLER" | "A-MODE" | "NEEDLE"
const std::string XSTEP_DEF              =            "1.0";
const std::string A_MODE_SCANLINES_DEF   =              "1";
const std::string NEEDLEPOS_DEF          =            "0.0";
const std::string A_MODE_TXPAT_DEF       =  "5 MHz 1 Pulse";
const std::string DOPPLER_TXPAT_DEF      = "4 MHz 2 Pulses";
const std::string COMMENT_DEF	         =    "No comments";


std::string XPOSMIN                      = XPOSMIN_DEF;
std::string XPOSMAX                      = XPOSMAX_DEF;
std::string XSPEED                       = XSPEED_DEF;
std::string ZPOSMIN                      = ZPOSMIN_DEF;
std::string ZPOSMAX                      = ZPOSMAX_DEF;

std::string A_MODE_OFFSETMIN             = A_MODE_OFFSETMIN_DEF;
std::string A_MODE_OFFSETMAX             = A_MODE_OFFSETMAX_DEF;
std::string A_MODE_AUTOGAIN              = A_MODE_AUTOGAIN_DEF;
std::string A_MODE_MANUALGAIN            = A_MODE_MANUALGAIN_DEF;
std::string A_MODE_GAINRATE              = A_MODE_GAINRATE_DEF;
std::string A_MODE_FILTERTYPE            = A_MODE_FILTERTYPE_DEF;

std::string M_MODE_SCANTIME              = M_MODE_SCANTIME_DEF;

std::string DOPPLER_OFFSETMIN            = DOPPLER_OFFSETMIN_DEF;
std::string DOPPLER_OFFSETMAX            = DOPPLER_OFFSETMAX_DEF;
std::string DOPPLER_AUTOGAIN             = DOPPLER_AUTOGAIN_DEF;
std::string DOPPLER_MANUALGAIN           = DOPPLER_MANUALGAIN_DEF;
std::string DOPPLER_FILTERTYPE           = DOPPLER_FILTERTYPE_DEF;

std::string A_MODE_PORT                  = A_MODE_PORT_DEF;
std::string DOPPLER_PORT                 = DOPPLER_PORT_DEF;
std::string A_MODE_TXPAT                 = A_MODE_TXPAT_DEF;
std::string DOPPLER_TXPAT                = DOPPLER_TXPAT_DEF;
std::string DOPPLER_ANGLE                = DOPPLER_ANGLE_DEF;
std::string MODE                         = MODE_DEF;
std::string XSTEP                        = XSTEP_DEF;
std::string A_MODE_SCANLINES             = A_MODE_SCANLINES_DEF;
std::string DOPPLER_SCANLINES            = DOPPLER_SCANLINES_DEF;
std::string NEEDLEPOS                    = NEEDLEPOS_DEF;
std::string COMMENT                      = COMMENT_DEF;


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
float xposmin   =  10.0; //mm
float xposmax   =  50.0; //1/10 mm
float xspeed     = 10.0;   // 8 mm/s
float xstep = 1.0; //mm

float zposmin   = 0.0;
float zposmax   = 4.0;
float needlepos = 0.0;

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
