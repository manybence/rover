// gpio_handler.cpp
#include "gpio_handler.h"

void initializeGPIO()
{
    if (gpioInitialise() < 0)
        return;

    gpioSetMode(IO_DONE, PI_INPUT);
    gpioSetMode(IO_RESET, PI_OUTPUT);
    gpioSetMode(IO_RST, PI_OUTPUT);
    gpioSetMode(IO_HILO, PI_OUTPUT);
    gpioSetMode(IO_ICE_CS, PI_INPUT);
    gpioSetMode(IO_F_CS, PI_INPUT);
    gpioSetMode(IO_F_MOSI, PI_INPUT);
    gpioSetMode(IO_F_MISO, PI_INPUT);
    gpioSetMode(IO_F_SCLK, PI_INPUT);
}

void terminateGPIO()
{
    gpioTerminate();
}
