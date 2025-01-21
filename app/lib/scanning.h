#ifndef scanning_h
#define scanning_h

#include <string.h>
#include "serial_comm.h"
#include "fpga.h"
#include "flash.h"
#include "signal_processing.h"
#include "relay.h"
#include "file_handler.h"
#include <thread>

#include <openssl/evp.h>
#include <openssl/err.h>

#include <cstdio>
#include <vector>
#include <cstdint>

#include <iostream>
#include <fstream>
#include <iomanip>  // for std::setw, std::setfill, std::hex


#define DOPPLER_START_PULSE (10 * 128)
#define DOPPLER_END_PULSE   ((10 * 128) + 128)
#define A_MODE_START_PULSE    0
#define A_MODE_END_PULSE   7167
#define us 1000000
#define GuardTime 500000

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


//Turn this false for reading the initial image (readout verification)
static bool NORMAL_FPGA_OPERATION = true;




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

int moveToPos(float xpos) {

    // Move to a given X-position
    try {
        float curr_pos = ReadMotorPosition(); // Read current position
        float distance = abs(curr_pos - xpos);
        MoveMotorToPosition(xpos);
        int expectedtime_us = GuardTime + (distance * 1000000) / xspeed;
        std::this_thread::sleep_for(std::chrono::microseconds(expectedtime_us));
        return 0;
    }
    catch (const std::runtime_error& e) {
        std::cerr << "Runtime error: " << e.what() << std::endl;
        return -1;
    }
}

int processDopplerMode() {

    std::cout << "Debug: Starting processDopplerMode function\n";

    float xpos = 0.0;
    int idx, i, reps;

    int longest_run;
    int app;
    int newdacval;
    int _dacval, _offset, _hiloval;
    DACDeltaCalculator dacDelta;
    bool autogain;
    //ARRAY_SIZE is the full 7168 samples deep buffer - for doppler data
    std::vector<int16_t> raw_input_data_D(ARRAY_SIZE);

    std::vector<double_t> raw_input_data_d(ARRAY_SIZE);
    std::vector<int32_t> filtereddata(ARRAY_SIZE);
    std::vector<double> hilbertindata = {};
    DataBufferType dataBuffer;
    std::vector<std::vector<double>> dataarray;
    
    //printf("ActivatePort:%s [%d]\n", DOPPLER_PORT.c_str(),lookupString(DOPPLER_PORT, translationTable));
    ActivatePort(lookupString(DOPPLER_PORT, translationTable));

    std::ofstream debugFile("debug_output.txt");
     
    spiOpen();

	// Move to scanning position
	std::cout << "Moving to target position" << std::endl;
    moveToPos(xposmax); // Move to target pos

    // Start scanning
    auto then    = std::chrono::high_resolution_clock::now();
    auto then_us = std::chrono::duration_cast<std::chrono::microseconds>(then.time_since_epoch()).count();
    resetFPGA();
    dataarray.clear();
    autogain = stringToBool(parameters["DOPPLER_AUTOGAIN"]);
    printf("autogain value: %s\n", autogain ? "true" : "false");
    for (idx = offsetmin; idx < offsetmax/*STGHIGH*/; idx++) { //need to decide how HILO is controlled in HW

        _dacval  = dac_val[STRATEG[idx]]; //initial table guesses
        _offset  = DOPP_OF[STRATEG[idx]];
        _hiloval = hiloval[STRATEG[idx]];
        _dacval  = manualgain;

        if (NORMAL_FPGA_OPERATION && autogain) {
            for (reps = 0; reps < 6; reps++) {
                //Start a measurement
                fpga(txpat, _dacval, /*_hiloval*/0, _offset);//do this atleast once
                start_pulse = DOPPLER_START_PULSE;
                end_pulse = DOPPLER_END_PULSE;

                // empty dummy spi value
                read_fpga_line(start_pulse);
                
                //Reading the captured data from the FPGA
                for (i = start_pulse; i <= end_pulse; i++) {
                    raw_input_data_D[i] = read_fpga_line(i);
                };

                // Assume doppler mode in this section
                longest_run = findLongestRun(raw_input_data_D, start_pulse, end_pulse);   //use only some of the pulses to estimate longest run
                if (longest_run > 0) { //clipping detected
                    newdacval = _dacval + mapValue(longest_run);
                    if (newdacval > 1023) newdacval = 1023;
                    if (newdacval <    0) newdacval =    0;
                    _dacval = newdacval;
                } else {
                    app = findPeakValue(raw_input_data_D, start_pulse, end_pulse);        //use only some of the pulses to estimate max peak peak value
                    newdacval = _dacval + dacDelta(app);
                    if (newdacval > 1023) newdacval = 1023;
                    if (newdacval <    0) newdacval =    0;
                    _dacval = newdacval;
                }
              }
            }
            
            if (NORMAL_FPGA_OPERATION) 
            for (reps = 0; reps < lines; reps++) {
                if (NORMAL_FPGA_OPERATION) fpga(txpat, _dacval, _hiloval, _offset);
                auto now = std::chrono::high_resolution_clock::now();
                auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count() - then_us;

                readAllFpgaSamples(raw_input_data_D.data(),7168); 
                
                // Buffer the data in RAM
                dataBuffer.emplace_back(xpos, STRATEG[idx], microseconds, _dacval, _offset, raw_input_data_D);
        };
    };
    
	// Write all buffered data to CSV
   if (SAVE_AS_CSV) save_data(dataBuffer, false);
    spiClose();
   
   return 0;
}

int processAMode() {
	
	std::cout << "Debug: Starting process A-Mode function\n";
	
    float xpos = 0.0;
    int expectedtime_us;
    int _dacval, _offset, _hiloval;
    DACDeltaCalculator dacDelta;
    std::vector<int16_t> raw_input_data_A(A_MODE_BUFLEN);
    DataBufferType dataBuffer;
    std::vector<std::vector<double>> dataarray;
    InitMotorPosition();
    MotorSpeed(xspeed); // [mm/s] 
    //printf("ActivatePort:%s [%d]\n", A_MODE_PORT.c_str(),lookupString(A_MODE_PORT, translationTable));
    ActivatePort(lookupString(A_MODE_PORT, translationTable));

    spiOpen();

    // Move to starting position
	printf("Move to starting position:%10.2f\n", xposmin);
    moveToPos(xposmin); // Move to target pos
	printf(" - done -\n");

	// Start scanning motion
	std::cout << "Moving to target position" << std::endl;
    
	printf("Move to starting position:%10.2f\n", xposmax);
    MoveMotorToPosition(xposmax);
	printf(" - done -\n");

    float distance = abs(xposmax - xposmin);
    expectedtime_us = GuardTime + (distance * 1000000) / xspeed;
	printf("expectedtime_us:%d\n", expectedtime_us);

	// Configure FPGA
    _dacval = manualgain;
    _offset = 0;
    _hiloval = 0;
    
    // Start FPGA scanning
    std::cout << "Start Scanning" << std::endl;
    stopFlag = false;
    resetFPGA();
    if (NORMAL_FPGA_OPERATION) fpga(txpat, _dacval, _hiloval, _offset);
    auto then = std::chrono::high_resolution_clock::now();
    auto then_us = std::chrono::duration_cast<std::chrono::microseconds>(then.time_since_epoch()).count();
    while (!stopFlag) {
        dataarray.clear();
        if (NORMAL_FPGA_OPERATION) fpga_scan();
        auto now = std::chrono::high_resolution_clock::now();
        auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count() - then_us;
        if (expectedtime_us < microseconds) stopFlag = true;
        readAllFpgaSamples(raw_input_data_A.data(),4004); 

        // Buffer the data in RAM
        dataBuffer.emplace_back(xpos, _hiloval, microseconds, _dacval, _offset, raw_input_data_A);
        
    }

	// Get position log
    GetLog();

    // Write all buffered data to CSV
    if (SAVE_AS_CSV) save_data(dataBuffer, true);
    spiClose();
    return 0;
}

int processMMode(float xpos_target) {
	
	std::cout << "Debug: Starting process M-Mode function\n";
    int _dacval, _offset, _hiloval;
    DACDeltaCalculator dacDelta;
    std::vector<int16_t> raw_input_data_A(A_MODE_BUFLEN);
    DataBufferType dataBuffer;
    std::vector<std::vector<double>> dataarray;

    //printf("ActivatePort:%s [%d]\n", A_MODE_PORT.c_str(),lookupString(A_MODE_PORT, translationTable));
    ActivatePort(lookupString(A_MODE_PORT, translationTable));

    spiOpen();

	// Move to scanning position
    moveToPos(xpos_target); // Move to target pos

	// Configure FPGA
    _dacval = manualgain;
    _offset = 0;
    _hiloval = 0;
    
    // Start FPGA scanning
    stopFlag = false;
    std::cout << "Start Scanning" << std::endl;
    resetFPGA();
    if (NORMAL_FPGA_OPERATION) fpga(txpat, _dacval, _hiloval, _offset);
    auto then = std::chrono::high_resolution_clock::now();
    auto then_us = std::chrono::duration_cast<std::chrono::microseconds>(then.time_since_epoch()).count();
    while (!stopFlag) {
        dataarray.clear();
        if (NORMAL_FPGA_OPERATION) fpga_scan();
        auto now = std::chrono::high_resolution_clock::now();
        auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count() - then_us;
        if (scanning_time * 1000 < microseconds) stopFlag = true;
        readAllFpgaSamples(raw_input_data_A.data(),4004); 
      
        // Buffer the data in RAM
        dataBuffer.emplace_back(xpos_target, _hiloval, microseconds, _dacval, _offset, raw_input_data_A);
    }

	// Get position log
    GetLog();

    // Write all buffered data to CSV
    if (SAVE_AS_CSV) save_data(dataBuffer, false);
    spiClose();
    return 0;

}

int processMFullScan() {

    std::cout << "Debug: Starting process M-Mode full scan function\n";

    // Create range of Xpos values
    std::vector<float> xpos_values = generate_xpos_range();

    // Perform M-mode scan at each given XPOS
    int counter = 0;
    std::string logfilename = datfilename;
    for (float xpos_target : xpos_values) {
        std::cout << "Next target: " << xpos_target << std::endl;
        std::cout << "datfilename: " << datfilename << std::endl;
        datfilename = update_filename(logfilename, counter);
        processMMode(xpos_target); // Perform M-mode scan
        usleep(10000);
        counter++;
    }

    std::cout << "Full M-mode scan finished!" << std::endl;

    return 0;
}

int processNeedleMode(float depth) {
    std::cout << "Debug: Starting processNeedleMode function\n";
    InitMotorCommunication();
    NeedleMotorPosition(depth);
    return 0;
}

int processJustReadBuffer() {
    const char* expected_hash = "1dbcb5827afb64bd83c5f1f8631ffc50febbe367f6fdb8db69aea82db938c4b3";
    std::vector<int16_t> raw_input_data(ARRAY_SIZE);

    int result = -1;
    spiOpen();
     
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    if (!mdctx) {
        fprintf(stderr, "Error creating EVP_MD_CTX\n");
        goto cleanup;
    }

    if (1 != EVP_DigestInit_ex(mdctx, EVP_sha256(), NULL)) {
        fprintf(stderr, "Error initializing SHA-256\n");
        goto cleanup;
    }
    readAllFpgaSamples(raw_input_data.data(),7168);
    EVP_DigestUpdate(mdctx, raw_input_data.data(), 7168);
    {
        unsigned char hash[EVP_MAX_MD_SIZE];
        unsigned int hash_len;

        if (1 != EVP_DigestFinal_ex(mdctx, hash, &hash_len)) {
            fprintf(stderr, "Error finalizing hash\n");
            goto cleanup;
        }

        // Convert the hash to a hex string
        char hash_string[EVP_MAX_MD_SIZE*2 + 1];
        for (unsigned int j = 0; j < hash_len; j++) {
            sprintf(&hash_string[j*2], "%02x", hash[j]);
        }
        hash_string[hash_len*2] = '\0';

        // Compare with expected
        if (strcmp(hash_string, expected_hash) == 0) {
            //printf("Hash matches the expected value!\n");
            result = 0;
        } else {
            printf("Computed checksum of the initial FPGA mem image does NOT match the expected value.\n");
            printf("Computed SHA256:%s\n", hash_string);
            printf("Expected SHA256:%s\n", expected_hash);
        }
    }

cleanup:
    if (mdctx) EVP_MD_CTX_free(mdctx);
    spiClose();
    return result;    
}



#endif
