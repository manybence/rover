#ifndef relay_h
#define relay_h

#include <unistd.h>
#include "gpio_handler.h"
#include "parameters.h"
#include "defaults.h"
#include "diverse.h"

//DELAYRELAY 30000 should be enough, but sometimes not
#define DELAYRELAY 60000

#define ONESEC 1000000
#define MS200   200000
#define MS1       1000
#define MS70     70000
#define MS21     21000

#define SPIRATE 2200000 //approx. MAX Functional value

void ActivateRelayCoil(int c){
	gpioWrite(c, 1);
	usleep(DELAYRELAY);
	gpioWrite(c, 0);
	usleep(DELAYRELAY);
}

void ConfigRelays(int c1, int c2){
	ActivateRelayCoil(c1);
	ActivateRelayCoil(c2);
}

void ActivatePort(int x) {
    switch(x) {
	case PORT_X4:
			ConfigRelays(IO_P1, IO_P4); //K300(S), K301(R)
			break;

        case PORT_X3:
			ConfigRelays(IO_P1, IO_P3); //K300(S), K301(S)
			break;

        case PORT_X2:
			ConfigRelays(IO_P2, IO_P4); //K300(R), K301(R)
			break;

        case PORT_X1:
			ConfigRelays(IO_P2, IO_P3); //K300(R), K301(S)
			break;

        default:
           break;
	   }
}

void TestPorts(int p1, int p2){
	ActivatePort(p1);
	ActivatePort(p2);
}

void InitHW() {
        
    printf("Testing relays...\n");
    std::string port;
    initializeGPIO();
    gpioWrite(IO_EN3V3, 1); //turn on 3V3 rail
    gpioWrite(IO_EN5V,  1); //turn on 5V rail

    //Exercise ports (toggle Relay K300)
    TestPorts(PORT_X3, PORT_X1);
    usleep(ONESEC);

    //Exercise ports (toggle Relay K301)
    TestPorts(PORT_X4, PORT_X3);
    usleep(ONESEC);
    
    // Exercise ports
    TestPorts(PORT_X1, PORT_X4);
    usleep(ONESEC);

    if (compareStrings(MODE, "A-MODE")) {port = A_MODE_PORT;};
    if (compareStrings(MODE,"DOPPLER")) {port = DOPPLER_PORT;};

    ActivatePort(lookupString(port, translationTable));

    usleep(MS1);
    gpioWrite(IO_FILSEL,filsel); //select filter 1=BPF, 0=HPF

    if (gpioRead(IO_DONE) == 0) {
        gpioWrite(IO_RESET, 0); // Power on reset
        usleep(MS1);
        gpioWrite(IO_RESET, 1);
        usleep(MS70);
    };

    if (gpioRead(IO_DONE) == 1)
        printf("PASS: The FPGA is configured\n");
    else
        printf("FAIL: The FPGA could not be configured\n");

    gpioSetMode(IO_RST, PI_OUTPUT);
    gpioWrite(IO_RST, 0); // release HW

    gpioSetMode(IO_ICE_CS, PI_OUTPUT);
    gpioWrite(IO_ICE_CS, 1);

    gpioSetMode(IO_F_CS, PI_OUTPUT);
    gpioWrite(IO_F_CS, 1);

    spiOpen(0, SPIRATE, 0);
    usleep(MS200);
    usleep(5000000); //5 secs. Wait for power rail stabilization
}

void release_HW() {
    
    spiClose(h);
    usleep(100);
    gpioWrite(IO_EN5V, 0);
    gpioWrite(IO_RST, 0); 
    gpioWrite(IO_EN3V3, 0);
    terminateGPIO();
    usleep(100);
}
    

#endif
