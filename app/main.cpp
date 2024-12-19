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

    // Reset mode
    if (compareStrings(parameters["MODE"], "RESET")) {
        InitMotorCommunication();
        InitHW(false);
        InitMotorPosition();
        release_HW();
        return 0;
    }
    
    // Initialize motor
    InitMotorCommunication();
    configured = IsMotorZeroed();
    if (!configured) {InitMotorPosition();}

    // Initialize hardware
	printf("Initialisation of HW\n");
    InitHW(configured);
    MotorSpeed(xspeed); // [mm/s] 
    
    // Start scanning process
    if (compareStrings(parameters["MODE"], "DOPPLER")) {
        processDopplerMode();
    } else
    if (compareStrings(parameters["MODE"], "M-MODE")) {
        processMMode(xposmax);
    } else
    if (compareStrings(parameters["MODE"], "M-MODE FULL SCAN")) {
        processMFullScan();
    } else
    if (compareStrings(parameters["MODE"], "A-MODE")) {
        processAMode();
    } else
    if (compareStrings(parameters["MODE"], "NEEDLE")) {
        processNeedleMode(depth);
    } else {
        std::cout << "Invalid mode of operation " << std::endl;
    }

    // Release hardware
    release_HW();
    return 0;
}

