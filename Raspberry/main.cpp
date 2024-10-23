#include <signal.h>
#include "lib/serial_comm.h"
#include "lib/diverse.h"
#include "lib/motors.h"
#include "lib/scanning.h"


int main(int argc, char* argv[]) {
    
    // Read, process and save incoming parameters
    saveParameters(argc, argv);
    
    // Assign interrupt handlers
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    // Initialize motor
    InitMotorCommunication();
    InitMotorPosition();

    // Start scanning process
    if (compareStrings(MODE,"DOPPLER")) {
        return processDopplerMode();
    } else
    if (compareStrings(MODE,"M-MODE")) {
        return processMMode();
    } else
    if (compareStrings(MODE,"A-MODE")) {
        return processAMode();
    } else
    if (compareStrings(MODE,"NEEDLE")) {
        return processNeedleMode();
    } else {
        return -1;
    }
}

