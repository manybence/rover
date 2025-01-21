#ifndef relay_h
#define relay_h
#include <gpiod.h>

//DELAYRELAY 30000 should be enough, but sometimes not
#define DELAYRELAY 60000

#define ONESEC 1000000
#define MS200   200000
#define MS1       1000
#define MS70     70000
#define MS21     21000

#define SPIRATE 2200000 //approx. MAX Functional value

void ActivateRelayCoil(int c){
	setGPIOValue(c, 1);
	usleep(DELAYRELAY);
	setGPIOValue(c, 0);
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
    //int i;
    printf("Initializing HW\n");
    printf("Testing relays...\n");
    std::string port;

    // Example usage of setting GPIO values
//    setGPIOValue(IO_EN3V3, 1); // Turns on 3.3V rail
//    setGPIOValue(IO_EN5V, 1);  // Turns on 5V rail
//    printf("Wait 3 seconds for rails to stabilize...\n");
//    usleep(3 * ONESEC);

    //for (i = 0; i < 1; i++){
    //Exercise ports (toggle Relay K300)
    
    TestPorts(PORT_X3, PORT_X1);
    usleep(ONESEC);

    //Exercise ports (toggle Relay K301)
    TestPorts(PORT_X4, PORT_X3);
    usleep(ONESEC);
     
    // Exercise ports
    TestPorts(PORT_X1, PORT_X4);
    usleep(ONESEC);
   // }
    if (compareStrings(MODE, "A-MODE") || compareStrings(MODE,"M-MODE")) {port = A_MODE_PORT;};
    if (compareStrings(MODE,"DOPPLER")) {port = DOPPLER_PORT;};

    ActivatePort(lookupString(port, translationTable));

    usleep(MS1);
    setGPIOValue(IO_FILSEL, filsel);
    
    //printf("Now release the fpga....\n", done_status);
    //setGPIOValue(IO_RESET, 0);  
    //usleep(MS70);
    //setGPIOValue(IO_RESET, 1);  
    //usleep(10*MS70); //Long time to measure the done pin
  
    // Read a GPIO value
    int done_status = getGPIOValue(IO_DONE);
    if (done_status == 1)
      printf("FPGA has booted: PASS\n");
    else  
      printf("FPGA has booted: FAIL\n");

    //if (digitalRead(IO_DONE) == HIGH)
    //    printf("PASS: The FPGA is configured\n");
    //else
    //    printf("FAIL: The FPGA could not be configured\n");
    
 //   pinMode(IO_RST, OUTPUT);
 //   digitalWrite(IO_RST, LOW);

 //   pinMode(IO_ICE_CS, OUTPUT);
 //   digitalWrite(IO_ICE_CS, HIGH);

 //   pinMode(IO_F_CS, OUTPUT);
 //   digitalWrite(IO_F_CS, HIGH);

    //wiringPiSPISetup(0, SPIRATE);
    //spiOpen(0, SPIRATE, 0);
    usleep(MS200);
    usleep(5000000); //5 secs. Wait for power rail stabilization
}

void release_HW() {
    
    //spiClose(h);
//    usleep(100);
//    setGPIOValue(IO_EN3V3, 0);
//    setGPIOValue(IO_RST, 0);
//    setGPIOValue(IO_EN5V, 0);
    
//    usleep(100);
}
    

#endif
