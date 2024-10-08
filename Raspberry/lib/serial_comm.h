#ifndef serial_comm_h
#define serial_comm_h

// serial_comm.h
#pragma once
#include <termios.h>
#include <fcntl.h>
#include <cstring>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <stdarg.h>
#include <iomanip>
#include <linux/serial.h>
#include <filesystem>
#include <sstream>
#include <fstream>
#include <sys/ioctl.h>
#include "defaults.h"
#include "diverse.h"
#include "parameters.h"

#define BAUDRATE B115200
#define MODEMDEVICE "/dev/ttyACM0"

int errno;
int openSerialPort(const char *device);
void closeSerialPort(int fd);

int fd = -1;

std::string logfilename, datfilename, picfilename;

std::string generate_filename(const std::string& directory) {
    int counter = 0;
    std::ostringstream oss;
    do {
        oss.str("");
        oss << directory << "/log" << std::setw(4) << std::setfill('0') << counter << ".txt";
        counter++;
    } while (std::filesystem::exists(oss.str()));
    return oss.str();
}

void SendParametersToStream() {
    std::cout << "Parameters:"                                  << std::endl;
    std::cout << "XPOSMIN: "            << XPOSMIN              << std::endl;
    std::cout << "XPOSMAX: "            << XPOSMAX              << std::endl;
    std::cout << "ZPOSMIN: "            << ZPOSMIN              << std::endl;
    std::cout << "ZPOSMAX: "            << ZPOSMAX              << std::endl;
    std::cout << "A_MODE_OFFSETMIN: "   << A_MODE_OFFSETMIN     << std::endl;
    std::cout << "A_MODE_OFFSETMAX: "   << A_MODE_OFFSETMAX     << std::endl;
    std::cout << "A_MODE_AUTOGAIN: "    << A_MODE_AUTOGAIN      << std::endl;
    std::cout << "A_MODE_MANUALGAIN: "  << A_MODE_MANUALGAIN    << std::endl;
    std::cout << "A_MODE_GAINRATE: "    << A_MODE_GAINRATE      << std::endl;
    std::cout << "A_MODE_FILTERTYPE: "  << A_MODE_FILTERTYPE    << std::endl;
    std::cout << "DOPPLER_OFFSETMIN: "  << DOPPLER_OFFSETMIN    << std::endl;
    std::cout << "DOPPLER_OFFSETMAX: "  << DOPPLER_OFFSETMAX    << std::endl;
    std::cout << "DOPPLER_AUTOGAIN: "   << DOPPLER_AUTOGAIN     << std::endl;
    std::cout << "DOPPLER_MANUALGAIN: " << DOPPLER_MANUALGAIN   << std::endl;
    std::cout << "DOPPLER_FILTERTYPE: " << DOPPLER_FILTERTYPE   << std::endl;
    std::cout << "A-MODE PORT: "        << A_MODE_PORT          << std::endl;
    std::cout << "DOPPLER PORT: "       << DOPPLER_PORT         << std::endl;
    std::cout << "A-MODE TXPAT: "       << A_MODE_TXPAT         << std::endl;
    std::cout << "DOPPLER TXPAT: "      << DOPPLER_TXPAT        << std::endl;
    std::cout << "DOPPLER ANGLE: "      << DOPPLER_ANGLE        << std::endl;
    std::cout << "MODE: "               << MODE                 << std::endl;
    std::cout << "XSTEP: "              << XSTEP                << std::endl;
    std::cout << "A-MODE SCANLINES: "   << A_MODE_SCANLINES     << std::endl;
    std::cout << "DOPPLER SCANLINES: "  << DOPPLER_SCANLINES    << std::endl;
    std::cout << "NEEDLEPOS: "          << NEEDLEPOS            << std::endl;
    std::cout << "COMMENT: "            << COMMENT              << std::endl;
};

void GetParameters(int argc, char* argv[]) {
	
    int i = 1;
    
    // Override defaults with command-line arguments
    if (argc > 26) {
        XPOSMIN              = std::string(argv[i++]);
        XPOSMAX              = std::string(argv[i++]);
        ZPOSMIN              = std::string(argv[i++]);
        ZPOSMAX              = std::string(argv[i++]);
        A_MODE_OFFSETMIN     = std::string(argv[i++]);
        A_MODE_OFFSETMAX     = std::string(argv[i++]);
        A_MODE_AUTOGAIN      = std::string(argv[i++]);
        A_MODE_MANUALGAIN    = std::string(argv[i++]);
        A_MODE_GAINRATE      = std::string(argv[i++]);
        A_MODE_FILTERTYPE    = std::string(argv[i++]);
        DOPPLER_OFFSETMIN    = std::string(argv[i++]);
        DOPPLER_OFFSETMAX    = std::string(argv[i++]);
        DOPPLER_AUTOGAIN     = std::string(argv[i++]);
        DOPPLER_MANUALGAIN   = std::string(argv[i++]);
        DOPPLER_FILTERTYPE   = std::string(argv[i++]);
        A_MODE_PORT          = std::string(argv[i++]);
        DOPPLER_PORT         = std::string(argv[i++]);
        A_MODE_TXPAT         = std::string(argv[i++]);
        DOPPLER_TXPAT        = std::string(argv[i++]);
        DOPPLER_ANGLE        = std::string(argv[i++]);
        MODE                 = std::string(argv[i++]);
        XSTEP                = std::string(argv[i++]);
        A_MODE_SCANLINES     = std::string(argv[i++]);
        DOPPLER_SCANLINES    = std::string(argv[i++]);
        NEEDLEPOS            = std::string(argv[i++]);
        COMMENT              = std::string(argv[i++]);
    };

    // Lookup strings in the table
    if (compareStrings(MODE,"A-MODE")) {
        printf("A-MODE\n");
        IsDopplerMode = false;
        short int lookupResult = lookupString(A_MODE_TXPAT, translationTable);
        txpat = lookupResult | BSCAN; // set lowest bit to signal B scan mode to the fpga
        lines = stringToInt(A_MODE_SCANLINES);
        offsetmin = stringToInt(A_MODE_OFFSETMIN);
        offsetmax = stringToInt(A_MODE_OFFSETMAX);
        filsel = lookupString(A_MODE_FILTERTYPE, translationTable);
        manualgain = stringToInt(A_MODE_MANUALGAIN);
        gainrate = stringToInt(A_MODE_GAINRATE);

    };

    if (compareStrings(MODE,"DOPPLER")) {
        printf("DOPPLER\n");
        IsDopplerMode = true;
        short int lookupResult = lookupString(DOPPLER_TXPAT, translationTable);
        txpat = lookupResult; // |DOPPL, but DOPPLER is 0 so no need to change lowest bit.
        lines = stringToInt(DOPPLER_SCANLINES);
        offsetmin = stringToInt(DOPPLER_OFFSETMIN);
        offsetmax = stringToInt(DOPPLER_OFFSETMAX);
        filsel = lookupString(DOPPLER_FILTERTYPE, translationTable);
        manualgain = stringToInt(DOPPLER_MANUALGAIN);
        angle = stringToInt(DOPPLER_ANGLE);
    };

    xposmin   = stringToFloat(XPOSMIN);
    xposmax   = stringToFloat(XPOSMAX);
    xstep     = stringToFloat(XSTEP);
    zposmin   = stringToFloat(ZPOSMIN);
    zposmax   = stringToFloat(ZPOSMAX);
    needlepos = stringToFloat(NEEDLEPOS);
};

int saveParameters(int argc, char* argv[]){
	
	// Read incoming parameters
    GetParameters(argc, argv);

    // Display parameters
    SendParametersToStream();

    // Ensure the data directory exists
    std::string directory = "data";
    if (!std::filesystem::exists(directory)) {
        std::cerr << "Debug: Creating directory " << directory << "\n";
        std::filesystem::create_directory(directory);
    };

    // Generate a unique filename
    std::cerr << "Debug: Generating filename\n";
    std::string filename = generate_filename(directory);
    logfilename = filename;
    datfilename = filename;
    picfilename = filename;
    replaceAll(datfilename, "\\log", "\\dat");
    replaceAll(datfilename, ".txt", ".csv");
    replaceAll(datfilename, "\\log", "\\pic");
    replaceAll(datfilename, ".txt", ".bmp");


    // Open the file
    std::cerr << "Debug: Opening file: " << logfilename << "\n";
    std::ofstream outfile(logfilename);
    if (!outfile) {
        std::cerr << "Error: Could not open file " << logfilename << " for writing\n";
        return 1;
    };

    // Redirect std::cout to the file
    std::cerr << "Debug: Redirecting stdout to file\n";
    std::streambuf* coutbuf = std::cout.rdbuf();
    std::cout.rdbuf(outfile.rdbuf());

    // Display parameters to the file
    SendParametersToStream();

    // Restore std::cout
    std::cout.rdbuf(coutbuf);
    std::cerr << "Debug: Restored stdout\n";
    return 0;

}

void pack_command(char command, float payload) {
	
	unsigned char tx_buffer[20];
    unsigned char *p_tx_buffer;
    p_tx_buffer = &tx_buffer[0];
    
    // Convert float value to byte array
    unsigned char payload_bytes[4];
    floatToByteArray(payload, payload_bytes);
    int length = sizeof(payload_bytes);
    
    *p_tx_buffer++ = '!';		// Start character
    *p_tx_buffer++ = command; 	// Command character
    *p_tx_buffer++ = length;		// Length of payload
    for (int i=0; i<length; i++) {
        *p_tx_buffer++ = payload_bytes[i];  //Payload bytes
    }
    *p_tx_buffer++ = '\n';		// End of Line

    if (fd != -1)
    {
		int count = write(fd, &tx_buffer[0], (p_tx_buffer - &tx_buffer[0])); // Filestream, bytes to write, number of bytes to write
        if (count < 0)
        {
            printf("UART TX error\n");
            return;
        }
    }

}

int set_interface_attribs(int fd, int speed, int parity) {
        
    struct termios tty;
    memset(&tty, 0, sizeof tty);
    
    if (tcgetattr(fd, &tty) != 0) {
        printf("Error from tcgeattr: %s\n", strerror(errno));
        return -1;
    }
    
    cfsetospeed(&tty, speed);
    cfsetispeed(&tty, speed);
    
    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit characters
    tty.c_iflag &= ~IGNBRK;                         // Ignore break signal
    tty.c_lflag = 0;                                // No canonical processing, no echo, no signaling chars    
    tty.c_oflag = 0;                                // No remapping, no delays
    
    tty.c_cc[VMIN] = 0;     // Read doesn't block
    tty.c_cc[VTIME] = 100;    // 10 sec read timeout
    
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);     // Shut off Xon/XOff control
    tty.c_cflag |= (CLOCAL | CREAD);            // Ignore modem control, enable reading
    tty.c_cflag &= ~(PARENB | PARODD);          // Shut off parity
    tty.c_cflag |= parity;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;
    
    
    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        printf("Error from tcsetattr: %s\n", strerror(errno));
        return -1;
    }
    return 0;
}

void InitMotorCommunication() {

    // open the device
    fd = open(MODEMDEVICE, O_RDWR | O_NOCTTY | O_SYNC);
    if (fd == -1)
    {
        perror(MODEMDEVICE);
        printf("Failed to open MODEMDEVICE \"/dev/ttyACM0\"\n");
        exit(-1);
    }

	struct serial_struct serinfo;
	ioctl(fd, TIOCGSERIAL, &serinfo);
	serinfo.xmit_fifo_size = 2*8192;  // Set to your desired size
	ioctl(fd, TIOCSSERIAL, &serinfo);
    set_interface_attribs(fd, BAUDRATE, 0); // set speed to BAUDRATE bps, 8n1 (no parity)
    tcflush(fd, TCIOFLUSH); // Discard both input and output data
}

std::string readMessage(int fd, int timeoutSec=10) {
    
    unsigned char rx_buffer[32000];

    // Set a timeout for the select function
    struct timeval timeout;
    timeout.tv_sec = timeoutSec;  // seconds
    timeout.tv_usec = 0; // 0 microseconds

    // Monitor the file descriptor for reading
    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(fd, &read_fds);

    // Wait until data is available to be read (or timeout occurs)
    int result = select(fd + 1, &read_fds, NULL, NULL, &timeout);
    
    if (result > 0) {  // Data is ready to be read
        if (FD_ISSET(fd, &read_fds)) {
            int rx_length = read(fd, rx_buffer, sizeof(rx_buffer) - 1); // Read incoming bytes

            if (rx_length > 0) {
                rx_buffer[rx_length] = '\0';  // Null-terminate the string
                return std::string(reinterpret_cast<char*>(rx_buffer)); // Return the received data
            } 
            else if (rx_length == -1) {
                std::cerr << "Error reading from fd." << std::endl;
                return "Reading error";
            }
        }
    } 
    else if (result == 0) {
        std::cerr << "Timeout: No data received within timeout." << std::endl;
        return "Timeout error";
    }
    

    return "";
}

#endif
