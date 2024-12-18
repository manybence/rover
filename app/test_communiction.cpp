#include "lib/serial_comm.h"
#include "lib/diverse.h"
#include "lib/motors.h"
#include <termios.h> 
#include <stdio.h>
#include <cstring>


int main(int argc, char* argv[]) {
    
    //float motorspeed = 13.9;   // [mm/s]
    //float motortarget = 62.4; // [mm]
    //float expectedtime_us = (motortarget * 1000000) / motorspeed;
    
    InitMotorCommunication();
    printf("fd opened: %d\n", fd);
    
    float pos = ReadMotorPosition();
    printf("Current XPOS: %f\n", pos);
}
