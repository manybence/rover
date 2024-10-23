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
    
    tty.c_cc[VMIN] = 1;     // Blocking mode (waiting until message arrives)
    tty.c_cc[VTIME] = 200;    // 20 sec read timeout
    
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);     // Shut off Xon/XOff control
    tty.c_cflag |= (CLOCAL | CREAD);            // Ignore modem control, enable reading
    tty.c_cflag &= ~(PARENB | PARODD);          // Shut off parity
    tty.c_cflag |= parity;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;
    
    // Setting the given attributes
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

std::string readResponse() {
    
    unsigned char rx_buffer[32000];
    
    int length = read(fd, rx_buffer, sizeof(rx_buffer));  // Read will timeout after 20 seconds if no data is available
    if (length < 0) {
        return "Error reading from serial port";
    } else if (length == 0) {
        return "Timeout error";
    } else {
        rx_buffer[length] = '\0';  // Null-terminate the string
        return std::string(reinterpret_cast<char*>(rx_buffer)); // Return the received data
    }
    
    return "";
}

#endif
