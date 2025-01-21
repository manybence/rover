#ifndef FLASH_H
#define FLASH_H

#include <openssl/evp.h>    // For EVP functions
#include <openssl/err.h>    // For error handling

#define IMAGESIZE 104156  // The expected size of the fpga binary imagefile
#define FLASH_ID 0x1130ef // Vendor and type specific identifier
// Global SPI1 file descriptor
int spi1_fd = -1;

// Function to open and configure the SPI1 device
int spi1Open(const char* device = "/dev/spidev0.1", uint8_t mode = SPI_MODE_0, uint8_t bits_per_word = 8, uint32_t speed = 1600000) {//32000000
    spi1_fd = open(device, O_RDWR);
    if (spi1_fd < 0) {
        std::cerr << "Error opening SPI1 device: " << strerror(errno) << std::endl;
        return -1;
    }

    int ret;
    // Set SPI1 mode
    ret = ioctl(spi1_fd, SPI_IOC_WR_MODE, &mode);
    if (ret < 0) {
        std::cerr << "Can't set SPI1 mode: " << strerror(errno) << std::endl;
        close(spi1_fd);
        spi1_fd = -1;
        return -1;
    }

    // Set bits per word
    ret = ioctl(spi1_fd, SPI_IOC_WR_BITS_PER_WORD, &bits_per_word);
    if (ret < 0) {
        std::cerr << "Can't set bits per word: " << strerror(errno) << std::endl;
        close(spi1_fd);
        spi1_fd = -1;
        return -1;
    }

    // Set max speed
    ret = ioctl(spi1_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
    if (ret < 0) {
        std::cerr << "Can't set max speed: " << strerror(errno) << std::endl;
        close(spi1_fd);
        spi1_fd = -1;
        return -1;
    }

    return 0; // Success
}

// Function to perform SPI1 transfer
int spi1Xfer(const uint8_t* txBuf, uint8_t* rxBuf, size_t len) {
    if (spi1_fd < 0) {
        std::cerr << "SPI1 device not opened. Call spi1Open() first." << std::endl;
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

    int ret = ioctl(spi1_fd, SPI_IOC_MESSAGE(1), &spi_transfer);
    if (ret < 1) {
        std::cerr << "Error during SPI1 transfer: " << strerror(errno) << std::endl;
        return -1;
    }

    return ret; // Return the number of bytes transferred
}

// Function to close the SPI1 device
void spi1Close() {
    if (spi1_fd >= 0) {
        close(spi1_fd);
        spi1_fd = -1;
    }
}


const char* getManufacturerName(uint8_t manufacturerId) {
    switch (manufacturerId) {
        case 0xEF: return "Winbond Electronics";
        case 0x20: return "Micron Technology / STMicroelectronics";
        case 0xC2: return "Macronix International";
        case 0x01: return "Spansion (Cypress Semiconductor)";
        case 0xBF: return "SST (Microchip Technology)";
        case 0x1C: return "EON Silicon Solutions";
        case 0x1F: return "Atmel (Microchip Technology) / Adesto Technologies";
        case 0xC8: return "GigaDevice Semiconductor";
        case 0x9D: return "ISSI (Integrated Silicon Solution Inc.)";
        case 0x98: return "Toshiba Memory (Kioxia)";
        case 0x37: return "AMIC Technology";
        case 0x04: return "Fujitsu";
        case 0xEC: return "Samsung Electronics";
        case 0x89: return "Intel Corporation";
        case 0xB0: return "Sharp Electronics";
        case 0x97: return "Texas Instruments";
        default:   return "Unknown Manufacturer";
    }
}

const char* getMemoryType(uint8_t memoryType) {
    switch (memoryType) {
        case 0x20: return "Standard Serial NOR Flash";
        case 0x30: return "Serial Flash Memory";
        case 0x40: return "Enhanced Serial NOR Flash";
        case 0x70: return "Serial NAND Flash";
        // Add more cases based on manufacturer specifications
        default:   return "Unknown Memory Type";
    }
}

const char* getMemoryCapacity(uint8_t capacityId) {
    switch (capacityId) {
        case 0x11: return "1 Mbit";
        case 0x12: return "2 Mbit";
        case 0x13: return "4 Mbit";
        case 0x14: return "8 Mbit (1 MB)";
        case 0x15: return "16 Mbit (2 MB)";
        case 0x16: return "32 Mbit (4 MB)";
        case 0x17: return "64 Mbit (8 MB)";
        case 0x18: return "128 Mbit (16 MB)";
        case 0x19: return "256 Mbit (32 MB)";
        case 0x1A: return "512 Mbit (64 MB)";
        case 0x1B: return "1 Gbit (128 MB)";
        case 0x1C: return "2 Gbit (256 MB)";
        // Add more cases as needed
        default:   return "Unknown Capacity";
    }
}



void prepareSpi1Command(uint32_t address, uint8_t *txBuf) {
    // Ensure txBuf is large enough to hold the command and data
    if (!txBuf) return;

    // First byte is the read command
    txBuf[0] = 0x0B;

    // Populate the address bytes (A23-A16, A15-A8, A7-A0)
    txBuf[1] = (address >> 16) & 0xFF; // A23-A16
    txBuf[2] = (address >> 8) & 0xFF;  // A15-A8
    txBuf[3] = address & 0xFF;         // A7-A0

    // Optional: Add a dummy byte (if required by the SPI flash)
    txBuf[4] = 0x00;

    // Data bytes are initialized to zero or left unmodified
    // This is for the SPI read to receive the data into rxBuf
    //memset(&txBuf[5], 0, IMAGE_SIZE_);
}

void prepareSpi1CommandPageProgram(uint32_t address, uint8_t *txBuf) {
    // Ensure txBuf is large enough to hold the command and data
    if (!txBuf) return;

    // First byte is the Page Program command
    txBuf[0] = 0x02;

    // Populate the address bytes (A23-A16, A15-A8, A7-A0)
    txBuf[1] = (address >> 16) & 0xFF; // A23-A16
    txBuf[2] = (address >> 8) & 0xFF;  // A15-A8
    txBuf[3] = address & 0xFF;         // A7-A0
}


void set_fpga_in_reset_state() { 
    setGPIOValue(IO_RESET, 0);
    spi1Open();
    usleep(100);
}


bool flash_ID() {
    uint8_t txBuf[ARRAY_SIZE];
    uint8_t rxBuf[ARRAY_SIZE];
    int j, k;
    //set_fpga_in_reset_state();
    spi1Open();
    usleep(100);
    for (k = 0; k < 1; k++) {
        j = 0;
        txBuf[j++] = 0x9f; //read JEDIC ID
        txBuf[j++] = 0x00;
        txBuf[j++] = 0x00;
        txBuf[j++] = 0x00;
        spi1Xfer(txBuf, rxBuf, j);
        //printf("[%d] %x %x %x %x\n", k, rxBuf[0], rxBuf[1], rxBuf[2], rxBuf[3]);
        //printf("Manufacturer: %s\n", getManufacturerName(rxBuf[1]));
        //printf("Memory Type:  %s\n", getMemoryType(rxBuf[2]));
        //printf("Capacity:     %s\n", getMemoryCapacity(rxBuf[3]));
    };
    if (((rxBuf[3] << 16) | (rxBuf[2] << 8) | rxBuf[1]) != FLASH_ID) {
      printf("ERROR NO CONTACT OR UNKNOWN FLASH TYPE\n");
      return false;
    }  
    return true;
};

uint32_t flash_Read_Unique_ID_Number() {
    uint8_t txBuf[9];
    uint8_t rxBuf[9];
    int j, k;

    for (k = 0; k < 1; k++) {
        j = 0;
        txBuf[j++] = 0x4b; 
        txBuf[j++] = 0x00;
        txBuf[j++] = 0x00;
        txBuf[j++] = 0x00;
        txBuf[j++] = 0x00;
        txBuf[j++] = 0x00;
        txBuf[j++] = 0x00;
        txBuf[j++] = 0x00;
        txBuf[j++] = 0x00;
        spi1Xfer(txBuf, rxBuf, j);
    };       
    return ((rxBuf[5] << 24) | (rxBuf[6] << 16) | (rxBuf[7] << 8) | rxBuf[8]);
}

bool flash_Busy() {
    uint8_t txBuf[2];
    uint8_t rxBuf[2];
    int j, k;

    for (k = 0; k < 10000; k++) { //wait maximum 1 sec
        j = 0;
        txBuf[j++] = 0x05; 
        txBuf[j++] = 0x00;
        usleep(100);
        spi1Xfer(txBuf, rxBuf, j);
        if ((rxBuf[1] & 0x01) == 0x0) break;
    };       
    return ((rxBuf[1] & 0x01) == 0x01);
}

std::vector<uint8_t> flash_read() {
    uint8_t txBuf[5 + 16];
    uint8_t rxBuf[5 + 16];
    EVP_MD_CTX *sha256Ctx = EVP_MD_CTX_new();  // Create a new context
    const EVP_MD *sha256 = EVP_sha256();       // Fetch SHA-256 digest method
    int k;
 
    if (EVP_DigestInit_ex(sha256Ctx, sha256, NULL) != 1) {
        fprintf(stderr, "Error initializing SHA-256 context\n");
        EVP_MD_CTX_free(sha256Ctx);
        return {};
    }
    
    // Read flash and calculate SHA-256
    for (k = 0; k < 0x196D; k++) {
        prepareSpi1Command(k * 16, txBuf);
        spi1Xfer(txBuf, rxBuf, 5 + 16);

        // Update SHA-256 hash with the data bytes in this block
        if (EVP_DigestUpdate(sha256Ctx, &rxBuf[5], 16) != 1) {
            fprintf(stderr, "Error updating SHA-256 context\n");
            EVP_MD_CTX_free(sha256Ctx);
            return {};
        }
        /*
        if (k < 4) 
            printf("%05X: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n", 
                   k*16, 
                   rxBuf[5], rxBuf[6], rxBuf[7], rxBuf[8], rxBuf[9], rxBuf[10], rxBuf[11], rxBuf[12],
                   rxBuf[13], rxBuf[14], rxBuf[15], rxBuf[16], rxBuf[17], rxBuf[18], rxBuf[19], rxBuf[20]);
        */ 
    }
    k = 0x196D;
    prepareSpi1Command(k * 16, txBuf);
    spi1Xfer(txBuf, rxBuf, 5 + 16);

    // Update SHA-256 hash with the final block
    if (EVP_DigestUpdate(sha256Ctx, &rxBuf[5], 12) != 1) {
        fprintf(stderr, "Error updating SHA-256 context\n");
        EVP_MD_CTX_free(sha256Ctx);
        return {};
    }
    /*
    printf("%05X: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n", 
           k*16, 
           rxBuf[5], rxBuf[6], rxBuf[7], rxBuf[8], rxBuf[9], rxBuf[10], rxBuf[11], rxBuf[12],
           rxBuf[13], rxBuf[14], rxBuf[15], rxBuf[16]);
    */
    // Finalize SHA-256 calculation
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hashLength = 0;

    if (EVP_DigestFinal_ex(sha256Ctx, hash, &hashLength) != 1) {
        fprintf(stderr, "Error finalizing SHA-256 hash\n");
        EVP_MD_CTX_free(sha256Ctx);
        return {};
    }

    // Free the SHA-256 context
    EVP_MD_CTX_free(sha256Ctx);

    // Print the final SHA-256 checksum
    //printf("SHA256 of flash: ");

    //std::cout << "flash SHA-256:";
    //for (unsigned int i = 0; i < hashLength; i++) {
    //    printf("%02x", hash[i]);
    //}
    //printf("\n");

    // Return the hash as a vector
    return std::vector<uint8_t>(hash, hash + hashLength);
}

int flash_write() {
    const char *filename = "RAPID_iCE40UP5KSG48.bin";        
    // Open the file in binary mode
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return EXIT_FAILURE;
    }

    // Read the file into a buffer
    std::vector<uint8_t> imagebuffer(IMAGESIZE);
    file.read(reinterpret_cast<char*>(imagebuffer.data()), IMAGESIZE);
    if (!file) {
        std::cerr << "Error reading file or file size is smaller than expected." << std::endl;
        return EXIT_FAILURE;
    }
    file.close();

    // Initialize OpenSSL algorithms (if needed)
    OpenSSL_add_all_algorithms();

    // Create and initialize the context
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    if (mdctx == nullptr) {
        std::cerr << "Error creating EVP_MD_CTX." << std::endl;
        ERR_print_errors_fp(stderr);
        return EXIT_FAILURE;
    }

    const EVP_MD *md = EVP_sha256();

    if (1 != EVP_DigestInit_ex(mdctx, md, nullptr)) {
        std::cerr << "Error initializing digest." << std::endl;
        ERR_print_errors_fp(stderr);
        EVP_MD_CTX_free(mdctx);
        return EXIT_FAILURE;
    }

    if (1 != EVP_DigestUpdate(mdctx, imagebuffer.data(), imagebuffer.size())) {
        std::cerr << "Error updating digest." << std::endl;
        ERR_print_errors_fp(stderr);
        EVP_MD_CTX_free(mdctx);
        return EXIT_FAILURE;
    }

    unsigned char fileHash[EVP_MAX_MD_SIZE];
    unsigned int hash_len;

    if (1 != EVP_DigestFinal_ex(mdctx, fileHash, &hash_len)) {
        std::cerr << "Error finalizing digest." << std::endl;
        ERR_print_errors_fp(stderr);
        EVP_MD_CTX_free(mdctx);
        return EXIT_FAILURE;
    }

    EVP_MD_CTX_free(mdctx);

    // Clean up OpenSSL (if needed)
    EVP_cleanup();

    // Print the SHA256 checksum of the file
    //std::cout << "file SHA-256: ";
    //for (unsigned int i = 0; i < hash_len; ++i) {
    //    printf("%02x", fileHash[i]);
    //}
    //std::cout << std::endl;
     

    // Compute SHA256 of the flash
    std::vector<uint8_t> flashHash = flash_read();

    if (flashHash.empty()) {
        std::cerr << "Error computing SHA256 of flash." << std::endl;
        return EXIT_FAILURE;
    }

    // Compare the two hashes
    bool hashes_equal = (hash_len == flashHash.size()) && std::equal(fileHash, fileHash + hash_len, flashHash.begin());

    if (hashes_equal) {
        std::cout << "fpga configuration is current." << std::endl;
        return EXIT_SUCCESS;
    } else {
        std::cout << "fpga configuration changed." << std::endl;
    }

    // Proceed to erase and program the flash
    uint8_t txBuf[ARRAY_SIZE];
    uint8_t rxBuf[ARRAY_SIZE];
    int j, k;
    
    spi1Open();

    // Erase the two first 64KB blocks
    for (k = 0x00; k < 0x02; k++) {
        j = 0;
        txBuf[j++] = 0x06; // Write Enable
        spi1Xfer(txBuf, rxBuf, j);
        
        j = 0;
        txBuf[j++] = 0xd8; // Block Erase 64K at a time
        txBuf[j++] = k;    // Start address
        txBuf[j++] = 0x00;
        txBuf[j++] = 0x00;
        spi1Xfer(txBuf, rxBuf, j);
        //printf("Block Erased [%06x..%06x]\n", (k << 16),((k+1) << 16)-1);
        flash_Busy();
        
        j = 0;
        txBuf[j++] = 0x04; // Write Disable
        spi1Xfer(txBuf, rxBuf, j);
    };
    
    flash_Busy();
 
    //printf("fpga uploading new configuration...\n");
    const size_t page_size = 256;
    const size_t total_size = imagebuffer.size();
    const size_t total_pages = (total_size + page_size - 1) / page_size;
    size_t address = 0;

    for (size_t k = 0; k < total_pages; k++) {
        size_t bytes_to_write = page_size;
        if (k == total_pages - 1 && (total_size % page_size != 0)) {
            // Last page, less than 256 bytes
            bytes_to_write = total_size % page_size; // Should be less than 256 in this case
        }

        // Write Enable
        txBuf[0] = 0x06;
        spi1Xfer(txBuf, rxBuf, 1);

        // Prepare Page Program command
        txBuf[0] = 0x02; // Page Program command
        txBuf[1] = (address >> 16) & 0xFF;
        txBuf[2] = (address >> 8) & 0xFF;
        txBuf[3] = address & 0xFF;

        // Copy data from imagebuffer to txBuf[4..]
        memcpy(&txBuf[4], &imagebuffer[address], bytes_to_write);

        if (bytes_to_write < page_size) {
            // Pad the rest with 0xFF (optional)
            memset(&txBuf[4 + bytes_to_write], 0xFF, page_size - bytes_to_write);
        }

        // Send the data
        spi1Xfer(txBuf, rxBuf, 4 + page_size);
        flash_Busy();

        // Write Disable
        txBuf[0] = 0x04;
        spi1Xfer(txBuf, rxBuf, 1);

        // Update address
        address += bytes_to_write;
    }
    //printf("fpga configuration updated\n");

    return 0;
}

int processConfiguration() {
    //set_fpga_in_reset_state();//fpga must be disabled to let RPi take over
    // std::cout << "Read FLASH.." << std::endl;
    //printf("flash ID : %6x",flash_ID());
    //printf("Unique ID:    %8x\n", flash_Read_Unique_ID_Number());
    flash_read();
    //std::cout << "write FLASH.." << std::endl;
    flash_write();
    //std::cout << "..DONE" << std::endl;
    setGPIOValue(IO_EN3V3, 0);
    setGPIOValue(IO_EN5V, 0);
    spi1Close();
    usleep(1000);
    setGPIOValue(IO_EN3V3, 1);
    setGPIOValue(IO_EN5V, 1);
    setGPIOValue(IO_RESET, 1);
    usleep(400000); //fpga should reboot now
    return 0;
}


#endif // FLASH_H
