#ifndef scanning_h
#define scanning_h

#include <string.h>
#include "serial_comm.h"
#include "fpga.h"
#include "signal_processing.h"
#include "bmp_gen.h"
#include "relay.h"

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

//A-MODE 90 deg., DMax=63.7 mm, fsam = 60 MHz, BufferLength = 4004
#define A_MODE_BUFLEN 4004
#define A_MODE_READOUT_OFFSET 960

int start_pulse;
int end_pulse;
volatile bool stopFlag = false;


const int ZRANGE =      672; // 16 bit samples x 7168 maps to 67.2 mm at 100 um resolution
const int XRANGE =  38 * 10; // each pixel represent 100 um resolution (current small restricted movement of scanhead)
RGB_data BModebuffer[ZRANGE][XRANGE];


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
    int idx, i, x, reps;
    int longest_run;
    int app;
    int newdacval;
    int _dacval, _offset, _hiloval;
    char txBuf[ARRAY_SIZE];
    char rxBuf[ARRAY_SIZE];
    DACDeltaCalculator dacDelta;
    bool autogain;

    //ARRAY_SIZE is the full 7168 samples deep buffer - for doppler data
    std::vector<int16_t> raw_input_data(ARRAY_SIZE);
    std::vector<double_t> raw_input_data_d(ARRAY_SIZE);
    std::vector<int32_t> filtereddata(ARRAY_SIZE);
    std::vector<double> hilbertindata = {};
    std::cerr << "Debug: Starting processDopplerMode function\n";
    // Parameters with default values

    printf("main loop begins\n");
    // Buffer to store raw input data
    std::vector<std::tuple<float, int, long long, int, int, std::vector<int16_t>>> dataBuffer;

    MotorSpeed(10);            //[mm/s]
    MoveMotorToPosition(250);  //[to be adjustable in the UI]

    usleep(10000000);
    printf("Initialisation of HW\n");
    InitHW();
    //xstep = 1.0;
    std::vector<std::vector<double>> dataarray;
    auto then    = std::chrono::high_resolution_clock::now();
    auto then_us = std::chrono::duration_cast<std::chrono::microseconds>(then.time_since_epoch()).count();

    resetFPGA();
    dataarray.clear();
    //printf(DOPPLER_AUTOGAIN);
    autogain = stringToBool(DOPPLER_AUTOGAIN);
    printf("autogain value: %s\n", autogain ? "true" : "false");
    for (idx = offsetmin; idx < offsetmax/*STGHIGH*/; idx++) { //need to decide how HILO is controlled in HW
        //printf("for idx:%2d\n", idx);

        //resetFPGA();
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
                txBuf[0] = (start_pulse >> 8) & 0xff;
                txBuf[1] = start_pulse & 0xff;
                spiXfer(h, txBuf, rxBuf, 2);

                //Reading the captured data from the FPGA
                for (i = start_pulse; i <= end_pulse; i++) {
                    txBuf[0] = (i >> 8) & 0xff;
                    txBuf[1] = i & 0xff;
                    spiXfer(h, txBuf, rxBuf, 2);
                    x = ((rxBuf[1] << 8) | (rxBuf[0] & 0xfc));
                    raw_input_data[i] = x;
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
                   // if (_dacval == 0) {_hiloval = 1; _dacval = 1023;}                   //reached max gain then try to get more gain if possible
                }
                printf("reps, dacval: %d %d\n", reps, _dacval);

              }
            }


            for (reps = 0; reps < lines; reps++) {
                fpga(txpat, _dacval, _hiloval, _offset);
                auto now = std::chrono::high_resolution_clock::now();
                auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count() - then_us;

                // empty dummy value
                txBuf[0] = (0 >> 8) & 0xff;
                txBuf[1] = 0 & 0xff;
                spiXfer(h, txBuf, rxBuf, 2);
                for (i = 0; i < ARRAY_SIZE; i++) {
                    txBuf[0] = (i >> 8) & 0xff;
                    txBuf[1] = i & 0xff;
                    spiXfer(h, txBuf, rxBuf, 2);
                    x = ((rxBuf[1] << 8) | (rxBuf[0] & 0xfc));
                    raw_input_data[i] = x;
                };
                // Buffer the data in RAM
                dataBuffer.emplace_back(xpos, STRATEG[idx], microseconds, _dacval, _offset, raw_input_data);
        };
    };
        // Write all buffered data to CSV
   if (SAVE_AS_CSV) {
        std::ofstream csvFile(datfilename);
        if (!csvFile.is_open()) {
            std::cerr << "Failed to open the CSV file for writing." << std::endl;
            return 1;
        }
        csvFile << "XPOS,STRATEG,TIME,DACVAL,OFFSET";
        for (size_t i = 0; i < ARRAY_SIZE; ++i) {
            csvFile << ",D[" << i << "]";
        }
        csvFile << "\n";
        for (const auto& entry : dataBuffer) {
            csvFile << 0 << "," << std::get<1>(entry) << "," << std::get<2>(entry) << "," << std::get<3>(entry) << "," << std::get<4>(entry);
            const auto& rawData = std::get<5>(entry);
            for (size_t i = 0; i < ARRAY_SIZE; ++i) {
                csvFile << "," << rawData[i];
            }
            csvFile << "\n";
        }
        csvFile.close();
        std::cout << "CSV file written successfully." << std::endl;
   }
   
   // Release hardware
   release_HW();
   
   return 0;
}

int processAMode() {
    float xpos = 0.0;
    int i, x, buf_adr;
    int expectedtime_us;
    int _dacval, _offset, _hiloval;
    int motorspeed;
    int motortarget;
    char txBuf[ARRAY_SIZE];
    char rxBuf[ARRAY_SIZE];
    DACDeltaCalculator dacDelta;
    memset(BModebuffer, 0, sizeof(BModebuffer));

    //A_MODE_BUFLEN is a the 4004 samples deep buffer - enough to hold full depth A_MODE
    std::vector<int16_t> raw_input_data_A(A_MODE_BUFLEN);
    std::vector<double_t> raw_input_data_d_A(A_MODE_BUFLEN);
    std::vector<int32_t> filtereddata_A(A_MODE_BUFLEN);
    std::vector<double> hilbertindata = {};
    std::cerr << "Debug: Starting processAMode function\n";

    // Buffer to store raw input data
    std::vector<std::tuple<float, int, long long, int, int, std::vector<int16_t>>> dataBuffer;
    std::vector<std::vector<double>> dataarray;

	// Initialise hardware
    printf("Initialisation of HW");
    InitHW();

	// Start scanning motion
	std::cout << "Start Scanning" << std::endl;
	tcflush(fd, TCIOFLUSH); // Discard both input and output data
    motorspeed = 8;   // [mm/s]
    motortarget = 52.5; // [52.5 mm]
    MotorSpeed(motorspeed);//mm pr sec  ..
    MoveMotorToPosition(motortarget); //test - ok full range movement
    expectedtime_us = GuardTime + (motortarget * 1000000) / motorspeed;

	// Configure FPGA
    _dacval = manualgain;
    _offset = 0;//15; //Somehow the first bit is overwhelmed by tx ringing
    _hiloval = 0;
    auto then = std::chrono::high_resolution_clock::now();
    auto then_us = std::chrono::duration_cast<std::chrono::microseconds>(then.time_since_epoch()).count();
    
    // Start FPGA scanning
    resetFPGA();
    fpga(txpat, _dacval, _hiloval, _offset);
    while (!stopFlag) {
        dataarray.clear();
            fpga_scan();
            auto now = std::chrono::high_resolution_clock::now();
            auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count() - then_us;
            if (expectedtime_us < microseconds){
                stopFlag = true; // To end and display time results (test)
            }

            // Read first sample twice (dummy stuff in spi buffer need to be discarded so a read from any addres would do)
            buf_adr = A_MODE_READOUT_OFFSET;
            txBuf[0] = (buf_adr >> 8) & 0xff;
            txBuf[1] = buf_adr & 0xff;
            spiXfer(h, txBuf, rxBuf, 2);

            // Read captured line
            for (i = 0; i < A_MODE_BUFLEN; i++) {
                buf_adr = i + A_MODE_READOUT_OFFSET;
                txBuf[0] = (buf_adr >> 8) & 0xff;
                txBuf[1] = buf_adr & 0xff;
                spiXfer(h, txBuf, rxBuf, 2);
                x = ((rxBuf[1] << 8) | (rxBuf[0] & 0xfc));
                raw_input_data_A[i] = x;
            };
            // Buffer the data in RAM
            dataBuffer.emplace_back(xpos, _hiloval, microseconds, _dacval, _offset, raw_input_data_A);
       // };

    }

	// Release hardware
    release_HW();

	// Get position log
    std::cout << "Get Log" << std::endl;
    GetLog();

    // Write all buffered data to CSV
    std::cout << "Saving log to CSV file." << std::endl;
    if (SAVE_AS_CSV) {
		std::ofstream csvFile(datfilename);
        if (!csvFile.is_open()) {
            std::cerr << "Failed to open the CSV file for writing." << std::endl;
            return 1;
        }
        csvFile << "XPOS,STRATEG,TIME,DACVAL,OFFSET";
        if (IsDopplerMode) {
            for (size_t i = 0; i < ARRAY_SIZE; ++i) {
                csvFile << ",D[" << i << "]";
            }
            csvFile << "\n";
            float ip0;
            ip0 = -10000.0;
            for (const auto& entry : dataBuffer) {
                float tm = std::get<2>(entry) / 1000.0;
                float ip = interpolatePosition(tm);
                if (abs(ip0-ip) < 0.01) break;
                csvFile << ip << "," << std::get<1>(entry) << "," << std::get<2>(entry) << "," << std::get<3>(entry) << "," << std::get<4>(entry);
                const auto& rawData = std::get<5>(entry);
                for (size_t i = 0; i < ARRAY_SIZE; ++i) {
                    csvFile << "," << rawData[i];
                }
                csvFile << "\n";
                ip0 = ip;
            }
        } else { //A_MODE
            for (size_t i = 0; i < A_MODE_BUFLEN; ++i) {
                csvFile << ",D[" << i << "]";
            }
            csvFile << "\n";
            float ip0;
            ip0 = -10000.0;
            for (const auto& entry : dataBuffer) {
                float tm = std::get<2>(entry) / 1000.0;
                float ip = interpolatePosition(tm);
                if (abs(ip0-ip) < 0.01) break;
                csvFile << ip << "," << std::get<1>(entry) << "," << std::get<2>(entry) << "," << std::get<3>(entry) << "," << std::get<4>(entry);
                const auto& rawData = std::get<5>(entry);
                for (size_t i = 0; i < A_MODE_BUFLEN; ++i) {
                    csvFile << "," << rawData[i];
                }
                csvFile << "\n";
                ip0 = ip;
            }
        }

        csvFile.close();
        std::cout << "CSV file written successfully." << std::endl;

    }
    return 0;
}

int processNeedleMode() {
    std::cerr << "Debug: Starting processNeedleMode function\n";
    InitMotorCommunication();
    NeedleMotorPosition(needlepos);
    return 0;
}

#endif
