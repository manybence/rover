
// gpio_handler.h
#pragma once
#include <pigpio.h>

#define IO_RST 18    // GPIO 18    [J700p12]
#define IO_DONE 23   // GPIO 23
#define IO_RESET 25  // GPIO 25
#define IO_HILO 27   // GPIO 27    [J700p13]
#define IO_ICE_CS 8  // SPI0 CS0
#define IO_F_CS 7    // SPI0 CS1
#define IO_F_MOSI 10 // SPI0 MOSI
#define IO_F_MISO 9  // SPI0 MISO
#define IO_F_SCLK 11 // SPI0 SCLK

#define IO_EN5V 15   // GPIO 15    [J700p10]
#define IO_EN3V3 14  // GPIO 14    [J700p8]
 
#define IO_FILTERSELECT 17  // GPIO 17    [J700p11]
#define IO_P1 5      // GPIO 5     [J700p29]
#define IO_P2 13     // GPIO 13    [J700p33]
#define IO_P3 19     // GPIO 19    [J700p35]
#define IO_P4 26     // GPIO 26    [J700p37]
#define IO_FILSEL 17 // GPIO 17 [J700p11]


void initializeGPIO() {
	
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

void terminateGPIO() {
	gpioTerminate();
}
