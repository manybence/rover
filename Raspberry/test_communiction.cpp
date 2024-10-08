#include "lib/serial_comm.h"
#include "lib/diverse.h"
#include "lib/motors.h"
#include <termios.h> 
#include <stdio.h>
#include <cstring>


int main(int argc, char* argv[]) {
    
    float motorspeed = 15;   // [mm/s]
    float motortarget = 65; // [52.5 mm]
    float expectedtime_us = (motortarget * 1000000) / motorspeed;
    
    InitMotorCommunication();
    printf("fd opened: %d\n", fd);
    
    // Send command
    InitMotorPosition();
    MotorSpeed(motorspeed);
    MoveMotorToPosition(motortarget);
    
    usleep(expectedtime_us);
    GetLog();

}
