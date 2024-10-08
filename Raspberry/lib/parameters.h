#ifndef parameters_h
#define parameters_h

#include <map>

// Mode register
#define BSCAN  0b00000001 // bit 0 in the mode register select B-SCAN (and auto dac dec). mode[0] = 1 is B-SCAN mode
#define DOPPL  0b00000000 // bit 0 in the mode register select DOPPLER (and init dac setup). mode[0] = 0 is Doppler mode
#define TXPAT0 0b00000000 // default TX pattern 10 MHz 16 Cycles
#define TXPAT1 0b00000010 // 5 MHz 9 Cycles
#define TXPAT2 0b00000100 // 4 MHz 8 Cycles
#define TXPAT3 0b00000110 // 2.5 MHz 2 Cycles
#define TXPAT4 0b00001000 // 2.5 MHz 4 Cycles
#define TXPAT5 0b00001010 // 2 MHz 4 Cycles
#define TXPAT6 0b00001100 // Barker 13
#define TXPAT7 0b00001110 // test pattern 0 -1 zzz

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
	{"4 MHz 3 Pulses",   TXPAT3},
	{"2.5 MHz 5 Pulses", TXPAT4},
	{"2 MHz 4 Pulses",   TXPAT5},
	{"Barker 7",         TXPAT6},
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
