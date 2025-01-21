#ifndef FPGA_H
#define FPGA_H

#include "relay.h"
#include <linux/spi/spidev.h>

const uint32_t MBAUD = 1000000;

// Register addresses
#define REG_ADC_TR   0x20
#define REG_DAC_LO   0x40
#define REG_DAC_HI   0x60
#define REG_DAC_INC  0x80
#define REG_OFS      0xA0
#define REG_MODE     0xC0

uint8_t txBuf[ARRAY_SIZE];
uint8_t rxBuf[ARRAY_SIZE];

//std::vector<int16_t> raw_input_data(ARRAY_SIZE);
//static const int TOTAL_WORDS = 7168;

// A placeholder for your final storage
//static short int raw_input_data_Debug[TOTAL_WORDS];

// Global SPI file descriptor
int spi_fd = -1;

// Function to open and configure the SPI device                                                                        
int spiOpen(const char* device = "/dev/spidev0.0", uint8_t mode = SPI_MODE_3, uint8_t bits_per_word = 8, uint32_t speed = 4000000) {//12800000 ok 3200000
    spi_fd = open(device, O_RDWR);
    if (spi_fd < 0) {
        std::cerr << "fpga.h: Error opening SPI device: " << strerror(errno) << std::endl;
        return -1;
    }

    int ret;
    // Set SPI mode
    ret = ioctl(spi_fd, SPI_IOC_WR_MODE, &mode);
    if (ret < 0) {
        std::cerr << "Can't set SPI mode: " << strerror(errno) << std::endl;
        close(spi_fd);
        spi_fd = -1;
        return -1;
    }

    // Set bits per word
    ret = ioctl(spi_fd, SPI_IOC_WR_BITS_PER_WORD, &bits_per_word);
    if (ret < 0) {
        std::cerr << "Can't set bits per word: " << strerror(errno) << std::endl;
        close(spi_fd);
        spi_fd = -1;
        return -1;
    }

    // Set max speed
    ret = ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
    if (ret < 0) {
        std::cerr << "Can't set max speed: " << strerror(errno) << std::endl;
        close(spi_fd);
        spi_fd = -1;
        return -1;
    }

    return 0; // Success
}

// Function to perform SPI transfer
int spiXfer(const uint8_t* txBuf, uint8_t* rxBuf, size_t len) {
    if (spi_fd < 0) {
        std::cerr << "SPI device not opened. Call spiOpen() first." << std::endl;
        return -1;
    }

    struct spi_ioc_transfer spi_transfer;
    memset(&spi_transfer, 0, sizeof(spi_transfer));

    spi_transfer.tx_buf = reinterpret_cast<uintptr_t>(txBuf);
    spi_transfer.rx_buf = reinterpret_cast<uintptr_t>(rxBuf);
    spi_transfer.len = len;
    spi_transfer.speed_hz = 0;        // Use default speed
    spi_transfer.bits_per_word = 0;   // Use default bits per word
    spi_transfer.delay_usecs = 0;     // No delay

    int ret = ioctl(spi_fd, SPI_IOC_MESSAGE(1), &spi_transfer);
    if (ret < 1) {
        std::cerr << "Error during SPI transfer: " << strerror(errno) << std::endl;
        return -1;
    }

    return ret; // Return the number of bytes transferred
}

// Function to close the SPI device
void spiClose() {
    if (spi_fd >= 0) {
        close(spi_fd);
        spi_fd = -1;
    }
}

void resetFPGA(){
   setGPIOValue(IO_RST, 1);
   usleep(10);
   setGPIOValue(IO_RST, 0);
   usleep(90);
}

void fpga(char scantype, int gain, int hilo, int offset) {
    int g, j;
    uint8_t s;
    uint8_t r;
    j = 0;
    txBuf[j++] = REG_MODE;          // Set MODE       0x6<<5 [MODE]
    txBuf[j++] = TXPAT2 | 0x01; //(scantype | 0x01); // even = DOPPL, odd = BSCAN choosing Doppler mode briefly to get DAC value set early

    g = gain;
    s = g & 0xff;
    r = (g >> 8) & 0x03;

    txBuf[j++] = REG_DAC_LO;    // Set Gain     0x2<<5 [DAC_LO]
    txBuf[j++] = s;
    txBuf[j++] = REG_DAC_HI;    //              0x3<<5 [DAC_HI]
    txBuf[j++] = r;
    
    setGPIOValue(IO_HILO, hilo);

    spiXfer(txBuf, rxBuf, j);
    usleep(200);//10 should be enough - maby the HILO shift need more time

    j = 0;
    txBuf[j++] = REG_MODE;      // Set MODE       0x6<<5 [MODE]
    txBuf[j++] = txpat;         //(txpat | DOPPL); // even = DOPPL,  odd = BSCAN (txpat is allready encoded when parameters are read)
    txBuf[j++] = REG_OFS;       // Offset         0x5<<5 [OFS]
    txBuf[j++] = offset;

    txBuf[j++] = REG_DAC_INC;   // DAC Dec value
    txBuf[j++] = gainrate;      // to be subtracted from DAC (VGA input) value each DAC update 6400 ns (10 MHz probe, soft tissue)

    txBuf[j++] = REG_ADC_TR;    // Start ADC trig 0x1<<5 [ADC_TR]
    txBuf[j++] = 0x00;

    spiXfer(txBuf, rxBuf, j);
    if (IsDopplerMode) {
       usleep(MS21);            // if doppler 21 ms 44 preample + 56 pulses.
    } else {
        usleep(100);            // if not on doppler mode the 78 us is the time to record the echo from a pulse. Some 1.2 us to send the pulse.
    }
}

void fpga_scan() {
    int j;
    j = 0;
    txBuf[j++] = REG_ADC_TR; // Start ADC trig 0x1<<5 [ADC_TR]
    txBuf[j++] = 0x00;
    spiXfer(txBuf, rxBuf, j);
    if (IsDopplerMode) {
       usleep(MS21);//if doppler 21 ms 44 preample + 56 pulses.
    } else {
       usleep(80);//if not on doppler mode the 78 us is the time to record the echo from a pulse. Some 1.2 us to send the pulse.
    }
}

int read_fpga_line(int buf_adr) {
    int j, k;
    j = 0;
    for (k = 0; k < 2048; k = k + 1) {
        txBuf[j++] = ((buf_adr + k) >> 8) & 0xff;
        txBuf[j++] = (buf_adr + k) & 0xff;
    }
    spiXfer(txBuf, rxBuf, j);
    return ((rxBuf[1] << 8) | (rxBuf[0] & 0xff));
}

void dummy_fpga(int buf_adr) {
    int j;
    j = 0;
    txBuf[j++] = (buf_adr >> 8) & 0xff;
    txBuf[j++] = buf_adr & 0xff;
    spiXfer(txBuf, rxBuf, j);
}


void read_fpga_l(int buf_adr, int n) {
    int j, k;
    j = 0;
    txBuf[j++] = (buf_adr >> 8) & 0xff; // dummy read
    txBuf[j++] = buf_adr & 0xff;
    spiXfer(txBuf, rxBuf, j);
    j = 0;
    for (k = 1; k <= n; k = k + 1) {
        txBuf[j++] = ((buf_adr + k) >> 8) & 0xff;
        txBuf[j++] = (buf_adr + k) & 0xff;
    }
    spiXfer(txBuf, rxBuf, j);
}

void readAllFpgaSamples(short int* dest, int totalWords)
{
    // The FPGA SPI can only handle 2048 16-bit words per call
    const int CHUNK_SIZE = 2048;

    int offset = 0;
    while (offset < totalWords)
    {
        // Decide how big this chunk is
        int wordsThisChunk = (totalWords - offset < CHUNK_SIZE)
                               ? (totalWords - offset)
                               : CHUNK_SIZE;

        // Read this chunk from the FPGA
        read_fpga_l(offset, wordsThisChunk);

        // Parse the rxBuf (2 bytes per word)
        int j = 0;
        for (int i = offset; i < offset + wordsThisChunk; i++)
        {
            uint8_t l_byte = rxBuf[j++]; // low
            uint8_t h_byte = rxBuf[j++]; // high
            dest[i] = static_cast<short int>((h_byte << 8) | l_byte);
        }

        offset += wordsThisChunk;
    }
}

#endif // FPGA_H
