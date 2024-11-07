#include <signal.h>
#include "lib/serial_comm.h"
#include "lib/diverse.h"
#include "lib/motors.h"
#include "lib/scanning.h"
#include "lib/BITE.h"

float depth = 4.0;

int main(int argc, char* argv[]) {

    // Safety check via Built-in Test Equipment (BITE)
    //read_BITE();

    // Read, process and save incoming parameters
    saveParameters(argc, argv);
    
    // Assign interrupt handlers
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    // Initialize motor
    InitMotorCommunication();
    configured = IsMotorZeroed();
    if (!configured) {InitMotorPosition();}

    // Initialize hardware
	printf("Initialisation of HW\n");
    InitHW(configured);
    
    // Start scanning process
    if (compareStrings(parameters["MODE"], "DOPPLER")) {
        return processDopplerMode();
    } else
    if (compareStrings(parameters["MODE"], "M-MODE")) {
        return processMMode();
    } else
    if (compareStrings(parameters["MODE"], "A-MODE")) {
        return processAMode();
    } else
    if (compareStrings(parameters["MODE"], "NEEDLE")) {
        return processNeedleMode(depth);
    } else {
        return -1;
    }
}

