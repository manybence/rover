#include <stdlib.h>
#include <unistd.h>
#include <pigpio.h>
#include <chrono>
#include "gpio_handler.h"
#include <stdio.h>


int main() {

    printf("RESET FPGA\n");
    initializeGPIO();
    gpioSetMode(IO_RST, PI_OUTPUT);
    gpioWrite(IO_RST, 0); // release HW
    return 0;
}
