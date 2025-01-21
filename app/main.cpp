#include <signal.h>
#include "lib/gpio_handler.hpp"
#include "lib/serial_comm.h"
#include "lib/diverse.h"
#include "lib/motors.h"
#include "lib/scanning.h"
#include "lib/BITE.h"
#include "lib/flash.h"

#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>

float depth = 4.0;

int main(int argc, char* argv[]) {
    
    // read, process and save incoming parameters
    saveParameters(argc, argv);
    auto mode_of_operation = parameters["MODE"];

    initializeGPIO();
    usleep(1000000);
    
    // report Built-in Test results
    read_BITE();

    // maintain FPGA configuration
    Investigate_Flash_Configuration();
    if (!flash_ID()) return -1;
    processConfiguration();
    Start_normal_operation();
    InitHW();
    processJustReadBuffer();

    // assign interrupt handlers
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    // initialize motor driver communication channel, check firmware version number
    InitMotorCommunication();
    if (!CheckMotorVersion()) return 0;

    // Initialize motor
    configured = IsMotorZeroed();
    if (!configured) InitMotorPosition();
    MotorSpeed(xspeed);

    // Start scanning process
    if      (mode_of_operation == "DOPPLER")          processDopplerMode();
    else if (mode_of_operation == "M-MODE")           processMMode(xposmax);
    else if (mode_of_operation == "M-MODE FULL SCAN") processMFullScan();
    else if (mode_of_operation == "A-MODE")           processAMode();
    else if (mode_of_operation == "NEEDLE")           processNeedleMode(depth);
    else if (mode_of_operation == "RESET")            InitMotorPosition();
    else std::cout << "Invalid mode of operation: [" << mode_of_operation << "]" << std::endl;
   
    release_HW();
    return 0;
}

