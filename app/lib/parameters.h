#ifndef parameters_h
#define parameters_h

#include <map>
#include <string>

// Mode register
#define BSCAN  0b00000001 // bit 0 in the mode register select B-SCAN (and auto dac dec). mode[0] = 1 is B-SCAN mode
#define DOPPL  0b00000000 // bit 0 in the mode register select DOPPLER (and init dac setup). mode[0] = 0 is Doppler mode
#define TXPAT0 0b00000000 // TXPAT0 10 MHz 4 Pulses
#define TXPAT1 0b00000010 // TXPAT1 5 MHz 3 Pulses
#define TXPAT2 0b00000100 // TXPAT2 4 MHz 2 Pulses
#define TXPAT3 0b00000110 // TXPAT3 5 MHz 1 Pulse
#define TXPAT4 0b00001000 // TXPAT4 2.5 MHz 5 Pulses
#define TXPAT5 0b00001010 // TXPAT5 2 MHz 4 Pulses
#define TXPAT6 0b00001100 // TXPAT6 10 MHz 1 Pulse
#define TXPAT7 0b00001110 // TXPAT7 Test Pulse


#define PORT_X1 1
#define PORT_X2 2
#define PORT_X3 3
#define PORT_X4 4

#define FILT_BPF 1
#define FILT_HPF 0

#define MIN_VALUE -32768
#define MAX_VALUE  32768

// Define the translation table
std::map<std::string, short int> translationTable = {

	//TX Pattern
	{"10 MHz 4 Pulses",  TXPAT0},
	{"5 MHz 3 Pulses",   TXPAT1},
	{"4 MHz 2 Pulses",   TXPAT2},
	{"5 MHz 1 Pulse",    TXPAT3},
	{"2.5 MHz 5 Pulses", TXPAT4},
	{"2 MHz 4 Pulses",   TXPAT5},
	{"10 MHz 1 Pulse",   TXPAT6},
	{"Test Pulse",       TXPAT7},

	//Relay
	{"X1",  	        PORT_X1},
	{"X2",              PORT_X2},
	{"X3",              PORT_X3},
	{"X4",              PORT_X4},

    //Filter
	{"BPF",            FILT_BPF},
	{"HPF",            FILT_HPF}
};


#endif
