#ifndef GPIO_HANDLER_H
#define GPIO_HANDLER_H
#include <gpiod.h>

//pigpoid pin numbers
const int IO_FILSEL = 17;
const int IO_RST    = 18;
const int IO_HILO   = 27;
const int IO_DONE   = 23;
const int IO_RESET  = 25;
//const int IO_F_CS   =  7;
const int IO_EN3V3  = 14;
const int IO_EN5V   = 15;
const int IO_P1     =  5;
const int IO_P2     = 13;
const int IO_P3     = 19;
const int IO_P4     = 26;

//const int IO_MOSI   = 10;
//const int IO_MISO   =  9;
//const int IO_SCLK   = 11;
//const int IO_ICE_CS =  8;

void MB_OFF();
void Investigate_Flash_Configuration();
void Start_normal_operation();

int initializeGPIO();
void setGPIOValue(int line_number, int value);
int getGPIOValue(int line_number);
void cleanupGPIO();
void Release_RESET();
#endif // GPIO_HANDLER_H




