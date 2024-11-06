#ifndef defaults_h
#define defaults_h

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include "parameters.h"

// Default values
const std::string XPOSMIN_DEF            =            "0.0";
const std::string XPOSMAX_DEF            =           "51.0";
const std::string XSPEED_DEF             =            "8.0";
const std::string A_MODE_OFFSETMIN_DEF   =              "0";
const std::string A_MODE_OFFSETMAX_DEF   =              "2";
const std::string A_MODE_AUTOGAIN_DEF    =          " true";
const std::string A_MODE_MANUALGAIN_DEF  =           " 700";
const std::string A_MODE_GAINRATE_DEF    =            "105"; //105 For 10 MHz probe  //90 for 5 MHz probe
const std::string A_MODE_FILTERTYPE_DEF  =            "BPF";
const std::string M_MODE_SCANTIME_DEF    =           "3000";    // 3000 ms = 3 s
const std::string DOPPLER_OFFSETMIN_DEF  =              "5";
const std::string DOPPLER_OFFSETMAX_DEF  =             "50";
const std::string DOPPLER_AUTOGAIN_DEF   =          "false";
const std::string DOPPLER_MANUALGAIN_DEF =           "1023";
const std::string DOPPLER_FILTERTYPE_DEF =            "HPF";
const std::string DOPPLER_SCANLINES_DEF  =              "4";
const std::string A_MODE_PORT_DEF        =             "X1";
const std::string DOPPLER_PORT_DEF       =             "X3";
const std::string DOPPLER_ANGLE_DEF      =              "0";
const std::string MODE_DEF               =         "A-MODE"; //"DOPPLER" | "A-MODE" | "NEEDLE"
const std::string A_MODE_SCANLINES_DEF   =              "1";
const std::string A_MODE_TXPAT_DEF       = "10 MHz 4 Pulses";
const std::string DOPPLER_TXPAT_DEF      = "4 MHz 8 Pulses";
const std::string COMMENT_DEF	           =    "No comments";
const std::string IS_CONFIGURED_DEF      =              "0";  // 0: not configured  | 1: configured


std::string XPOSMIN                      = XPOSMIN_DEF;
std::string XPOSMAX                      = XPOSMAX_DEF;
std::string XSPEED                       = XSPEED_DEF;

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
std::string A_MODE_SCANLINES             = A_MODE_SCANLINES_DEF;
std::string DOPPLER_SCANLINES            = DOPPLER_SCANLINES_DEF;
std::string COMMENT                      = COMMENT_DEF;
std::string IS_CONFIGURED                = IS_CONFIGURED_DEF;

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
float xposmin   =   0.0; //mm
float xposmax   = 520.0; //1/10 mm
float xspeed     = 8.0;   // 8 mm/s
int filsel = 0;
int scanning_time = 3000;   // M-mode scanning (ms)
unsigned char txpat = TXPAT2;
int gainrate    = TISSUE10M;
const int ARRAY_SIZE = 7168;
bool IsDopplerMode = false;
int h;
bool configured = false;

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
