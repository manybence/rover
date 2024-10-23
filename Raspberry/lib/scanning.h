#ifndef scanning_h
#define scanning_h

#include <string.h>
#include "serial_comm.h"
#include "fpga.h"
#include "signal_processing.h"
#include "relay.h"
#include "file_handler.h"

#define DOPPLER_START_PULSE (10 * 128)
#define DOPPLER_END_PULSE   ((10 * 128) + 128)
#define A_MODE_START_PULSE    0
#define A_MODE_END_PULSE   7167
#define us 1000000
#define GuardTime 1000000

// The following assume a dopplerangle of 60deg
// So the distance from transducer to the skin is filled with skinlike acoustic head of 5mm which is equvivalent with an offset count of 9
// The limits is extended [6.0 .. 73.6] mm, standard [12.4 .. 39.6] mm.
#define CAP_DOPPLER_EXT_OFS_MIN  18
#define CAP_DOPPLER_STD_OFS_MIN  30
#define CAP_DOPPLER_STD_OFS_MAX  78
#define CAP_DOPPLER_EXT_OFS_MAX 138

#define SEND_DATA_VIA_ETH true
#define SAVE_AS_CSV       true
#define SAVE_AS_BMP       true
#define MOVEMOTOR         true
#define CAP_BSCAN         true
#define CAP_DOPPLER       true

int start_pulse;
int end_pulse;
volatile bool stopFlag = false;


const int ZRANGE =      672; // 16 bit samples x 7168 maps to 67.2 mm at 100 um resolution
const int XRANGE =  38 * 10; // each pixel represent 100 um resolution (current small restricted movement of scanhead)


class DACDeltaCalculator {
private:
    static const int k1 = -869;
    static const int k2 = 182;
    static constexpr int max_app = 65533;
    std::vector<double> logTable;

    void initializeLogTable(int resolution) {
        logTable.resize(resolution + 1);
        for (int i = 1; i <= resolution; ++i) {
            logTable[i] = std::log10(i * (max_app / static_cast<double>(resolution)));
        }
    }

public:
    DACDeltaCalculator(int resolution = 1000) {
        initializeLogTable(resolution);
    }

    int operator()(int app) {
        if (app <= 0 || app > max_app) {
            throw std::out_of_range("app value must be between 1 and 65533");
        }
        int index = app * logTable.size() / max_app;
        double logValue = logTable[index];
        return static_cast<int>((k1 + k2 * logValue));
    }
};

void signalHandler(int signum) {
    stopFlag = true;
}

int processDopplerMode() {
    float xpos = 0.0;
    int idx, i, reps;
    int longest_run;
    int app;
    int newdacval;
    int _dacval, _offset, _hiloval;
    DACDeltaCalculator dacDelta;
    bool autogain;

    //ARRAY_SIZE is the full 7168 samples deep buffer - for doppler data
    std::vector<int16_t> raw_input_data(ARRAY_SIZE);
    std::vector<double_t> raw_input_data_d(ARRAY_SIZE);
    std::vector<int32_t> filtereddata(ARRAY_SIZE);
    std::vector<double> hilbertindata = {};
    std::cerr << "Debug: Starting processDopplerMode function\n";
    
	// Initialize hardware
	printf("Initialisation of HW\n");
    InitHW();
    
	// Move to scanning position
	std::cout << "Moving to target position" << std::endl;
	tcflush(fd, TCIOFLUSH); // Discard both input and output data
    float motortarget = xposmax; // [mm]
    MotorSpeed(xspeed);//mm pr sec  ..
    MoveMotorToPosition(motortarget); //test - ok full range movement
    int expectedtime_us = GuardTime + (motortarget * 1000000) / xspeed;
    usleep(expectedtime_us);

	// Buffer to store raw input data
    DataBufferType dataBuffer;
    std::vector<std::vector<double>> dataarray;
    
    // Start scanning
    auto then    = std::chrono::high_resolution_clock::now();
    auto then_us = std::chrono::duration_cast<std::chrono::microseconds>(then.time_since_epoch()).count();
    resetFPGA();
    dataarray.clear();
    autogain = stringToBool(DOPPLER_AUTOGAIN);
    printf("autogain value: %s\n", autogain ? "true" : "false");
    for (idx = offsetmin; idx < offsetmax/*STGHIGH*/; idx++) { //need to decide how HILO is controlled in HW

        _dacval  = dac_val[STRATEG[idx]]; //initial table guesses
        _offset  = DOPP_OF[STRATEG[idx]];
        _hiloval = hiloval[STRATEG[idx]];
        _dacval  = manualgain;

        if (autogain) {
            for (reps = 0; reps < 6; reps++) {
                //Start a measurement
                fpga(txpat, _dacval, /*_hiloval*/0, _offset);//do this atleast once
                start_pulse = DOPPLER_START_PULSE;
                end_pulse = DOPPLER_END_PULSE;

                // empty dummy spi value
                read_fpga_line(start_pulse);
                
                //Reading the captured data from the FPGA
                for (i = start_pulse; i <= end_pulse; i++) {
                    raw_input_data[i] = read_fpga_line(i);
                };

                // Assume doppler mode in this section
                longest_run = findLongestRun(raw_input_data, start_pulse, end_pulse);   //use only some of the pulses to estimate longest run
                if (longest_run > 0) { //clipping detected
                    newdacval = _dacval + mapValue(longest_run);
                    if (newdacval > 1023) newdacval = 1023;
                    if (newdacval <    0) newdacval =    0;
                    _dacval = newdacval;
                } else {
                    app = findPeakValue(raw_input_data, start_pulse, end_pulse);        //use only some of the pulses to estimate max peak peak value
                    newdacval = _dacval + dacDelta(app);
                    if (newdacval > 1023) newdacval = 1023;
                    if (newdacval <    0) newdacval =    0;
                    _dacval = newdacval;
                }
              }
            }


            for (reps = 0; reps < lines; reps++) {
                fpga(txpat, _dacval, _hiloval, _offset);
                auto now = std::chrono::high_resolution_clock::now();
                auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count() - then_us;

                // Empty dummy value
                read_fpga_line(0);
                
                // Read data lines
                for (i = 0; i < ARRAY_SIZE; i++) {
                    raw_input_data[i] = read_fpga_line(i);
                };
                
                // Buffer the data in RAM
                dataBuffer.emplace_back(xpos, STRATEG[idx], microseconds, _dacval, _offset, raw_input_data);
        };
    };
    
	// Write all buffered data to CSV
   if (SAVE_AS_CSV) save_data(dataBuffer, false);
   
   // Release hardware
   release_HW();
   
   return 0;
}

int processAMode() {
	
	std::cerr << "Debug: Starting process A-Mode function\n";
	
    float xpos = 0.0;
    int i;
    int expectedtime_us;
    int _dacval, _offset, _hiloval;
    DACDeltaCalculator dacDelta;

    //A_MODE_BUFLEN is a the 4004 samples deep buffer - enough to hold full depth A_MODE
    std::vector<int16_t> raw_input_data_A(A_MODE_BUFLEN);
    DataBufferType dataBuffer;
    std::vector<std::vector<double>> dataarray;

	// Initialise hardware
    printf("Initialisation of HW");
    InitHW();

	// Start scanning motion
	std::cout << "Moving to target position" << std::endl;
	tcflush(fd, TCIOFLUSH); // Discard both input and output data
    MotorSpeed(xspeed);//mm pr sec  ..
    MoveMotorToPosition(xposmax); //test - ok full range movement
    expectedtime_us = GuardTime + (xposmax * 1000000) / xspeed;

	// Configure FPGA
    _dacval = manualgain;
    _offset = 0;//15; //Somehow the first bit is overwhelmed by tx ringing
    _hiloval = 0;
    
    // Start FPGA scanning
    std::cout << "Start Scanning" << std::endl;
    resetFPGA();
    fpga(txpat, _dacval, _hiloval, _offset);
    auto then = std::chrono::high_resolution_clock::now();
    auto then_us = std::chrono::duration_cast<std::chrono::microseconds>(then.time_since_epoch()).count();
    while (!stopFlag) {
        dataarray.clear();
            fpga_scan();
            auto now = std::chrono::high_resolution_clock::now();
            auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count() - then_us;
            if (expectedtime_us < microseconds){
                stopFlag = true;
            }

            // Read first sample twice (dummy stuff in spi buffer need to be discarded so a read from any addres would do)
            read_fpga_line(A_MODE_READOUT_OFFSET);

            // Read captured lines
            for (i = 0; i < A_MODE_BUFLEN; i++) {
                raw_input_data_A[i] = read_fpga_line(i + A_MODE_READOUT_OFFSET);
            };
            
            // Buffer the data in RAM
            dataBuffer.emplace_back(xpos, _hiloval, microseconds, _dacval, _offset, raw_input_data_A);
    }

	// Release hardware
    release_HW();

	// Get position log
    std::cout << "Get Log" << std::endl;
    GetLog();

    // Write all buffered data to CSV
    if (SAVE_AS_CSV) save_data(dataBuffer, true);
    
    return 0;
}

int processMMode() {
	
	std::cerr << "Debug: Starting process M-Mode function\n";
	
	float xpos = 0.0;
    int i;
    int expectedtime_us;
    int _dacval, _offset, _hiloval;
    DACDeltaCalculator dacDelta;

    //A_MODE_BUFLEN is a the 4004 samples deep buffer - enough to hold full depth A_MODE
    std::vector<int16_t> raw_input_data_A(A_MODE_BUFLEN);
   

    // Buffer to store raw input data
    DataBufferType dataBuffer;
    std::vector<std::vector<double>> dataarray;

	// Initialise hardware
    printf("Initialisation of HW");
    InitHW();

	// Move to scanning position
	std::cout << "Moving to target position" << std::endl;
	tcflush(fd, TCIOFLUSH); // Discard both input and output data
    MotorSpeed(xspeed);//mm pr sec  ..
    MoveMotorToPosition(xposmax); //test - ok full range movement
    expectedtime_us = GuardTime + (xposmax * 1000000) / xspeed;
    usleep(expectedtime_us);

	// Configure FPGA
    _dacval = manualgain;
    _offset = 0;//15; //Somehow the first bit is overwhelmed by tx ringing
    _hiloval = 0;
    
    // Start FPGA scanning
    std::cout << "Start Scanning" << std::endl;
    resetFPGA();
    fpga(txpat, _dacval, _hiloval, _offset);
    auto then = std::chrono::high_resolution_clock::now();
    auto then_us = std::chrono::duration_cast<std::chrono::microseconds>(then.time_since_epoch()).count();
    while (!stopFlag) {
        dataarray.clear();
            fpga_scan();
            auto now = std::chrono::high_resolution_clock::now();
            auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count() - then_us;
            if (scanning_time < microseconds){
                stopFlag = true; // To end and display time results (test)
            }

            // Read first sample twice (dummy stuff in spi buffer need to be discarded so a read from any addres would do)
            read_fpga_line(A_MODE_READOUT_OFFSET);

            // Read captured lines
            for (i = 0; i < A_MODE_BUFLEN; i++) {
                raw_input_data_A[i] = read_fpga_line(i + A_MODE_READOUT_OFFSET);
            };
            
            // Buffer the data in RAM
            dataBuffer.emplace_back(xpos, _hiloval, microseconds, _dacval, _offset, raw_input_data_A);
    }

	// Release hardware
    release_HW();

	// Get position log
    std::cout << "Get Log" << std::endl;
    GetLog();

    // Write all buffered data to CSV
    if (SAVE_AS_CSV) save_data(dataBuffer, false);
    return 0;

}

int processNeedleMode() {
    std::cerr << "Debug: Starting processNeedleMode function\n";
    InitMotorCommunication();
    NeedleMotorPosition(needlepos);
    return 0;
}

#endif
