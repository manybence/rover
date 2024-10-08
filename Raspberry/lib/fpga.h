#ifndef fpga_h
#define fpga_h

#include <unistd.h>
#include "gpio_handler.h"
#include "parameters.h"
#include "relay.h"

// Register addresses
#define REG_ADC_TR   0x20
#define REG_DAC_LO   0x40
#define REG_DAC_HI   0x60
#define REG_DAC_INC  0x80
#define REG_OFS      0xA0
#define REG_MODE     0xC0



void resetFPGA(){
   gpioWrite(IO_RST, 1); // Causes a reset of the FPGA - refresh for a new cycle (automatic scan)
   usleep(10);
   gpioWrite(IO_RST, 0);
   usleep(90);
}

void fpga(char scantype, int gain, int hilo, int offset) {
    char txBuf[ARRAY_SIZE];
    char rxBuf[ARRAY_SIZE];
    int g, j;
    unsigned char s;
    unsigned char r;
    j = 0;
    txBuf[j++] = REG_MODE;          // Set MODE       0x6<<5 [MODE]
    txBuf[j++] = TXPAT2 | 0x01; //(scantype | 0x01); // even = DOPPL, odd = BSCAN choosing Doppler mode briefly to get DAC value set early

    //printf("txBuf[1] : %2d\n", txBuf[1]);

    g = gain;
    s = g & 0xff;
    r = (g >> 8) & 0x03;

    txBuf[j++] = REG_DAC_LO;    // Set Gain     0x2<<5 [DAC_LO]
    txBuf[j++] = s;
    txBuf[j++] = REG_DAC_HI;    //              0x3<<5 [DAC_HI]
    txBuf[j++] = r;
    gpioWrite(IO_HILO, hilo);

    spiXfer(h, txBuf, rxBuf, j);
    usleep(200);//10 should be enough - maby the HILO shift need more time
    //usleep(20000);//10 should be enough - maby the HILO shift need more time

    j = 0;
    txBuf[j++] = REG_MODE;      // Set MODE       0x6<<5 [MODE]
    txBuf[j++] = txpat;         //(txpat | DOPPL); // even = DOPPL,  odd = BSCAN (txpat is allready encoded when parameters are read)
    txBuf[j++] = REG_OFS;       // Offset         0x5<<5 [OFS]
    txBuf[j++] = offset;

    txBuf[j++] = REG_DAC_INC;   // DAC Dec value
    txBuf[j++] = gainrate;      // to be subtracted from DAC (VGA input) value each DAC update 6400 ns (10 MHz probe, soft tissue)

    txBuf[j++] = REG_ADC_TR;    // Start ADC trig 0x1<<5 [ADC_TR]
    txBuf[j++] = 0x00;

    spiXfer(h, txBuf, rxBuf, j);
    if (IsDopplerMode) {
       usleep(MS21);            // if doppler 21 ms 44 preample + 56 pulses.
    } else {
        usleep(100);            // if not on doppler mode the 78 us is the time to record the echo from a pulse. Some 1.2 us to send the pulse.
    }
}

void fpga_scan() {
    char txBuf[ARRAY_SIZE];
    char rxBuf[ARRAY_SIZE];
    int j;
    j = 0;
    txBuf[j++] = REG_ADC_TR; // Start ADC trig 0x1<<5 [ADC_TR]
    txBuf[j++] = 0x00;
    spiXfer(h, txBuf, rxBuf, j);
    if (IsDopplerMode) {
       usleep(MS21);//if doppler 21 ms 44 preample + 56 pulses.
    } else {
       usleep(100);//if not on doppler mode the 78 us is the time to record the echo from a pulse. Some 1.2 us to send the pulse.
    }
}

#endif
