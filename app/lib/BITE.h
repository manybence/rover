#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include "gpio_handler.h"

#define I2C_DEVICE "/dev/i2c-1"
#define LTC2991_I2C_ADDRESS_1 0x48
#define LTC2991_I2C_ADDRESS_2 0x49
#define SINGLE_END 0.00030518
#define VFORMAT "%-3s %5s: %8.2f V\n"
#define TFORMAT "%-3s %5s: %8.2f C\n"


#define LTC2991_CH_EN_TRIGGER 0x01
#define NUM_REGISTERS 0x1D // Number of registers to read

#define BIT(n) (1U << (n))

#define LTC2991_V7_V8_EN BIT(7)
#define LTC2991_V5_V6_EN BIT(6)
#define LTC2991_V3_V4_EN BIT(5)
#define LTC2991_V1_V2_EN BIT(4)
#define LTC2991_T_INT_VCC_EN BIT(3)

void read_registers(int file) {
    uint8_t buffer[2];
    uint8_t reg;
    uint16_t value;

    for (reg = 0x00; reg <= NUM_REGISTERS; reg++) {

        // Write the register address to the I2C bus
        if (write(file, &reg, 1) != 1) {
            perror("Failed to write to the I2C bus");
            return;
        }

        // Read data from the I2C bus
        if (read(file, buffer, 2) != 2) {
            perror("Failed to read from the I2C bus");
            return;
        }

        // Combine the two bytes read into a single 16-bit value
        value = (buffer[0] << 8) | buffer[1];
        if (reg == 0x0A) printf(VFORMAT, "M10","5V0A", (value * SINGLE_END * 2.0));
        if (reg == 0x0C) printf(VFORMAT, "M6", "8V6",  (value * SINGLE_END * 3.495));
        if (reg == 0x0E) printf(VFORMAT, "M5", "3V3D", (value * SINGLE_END * 1.287));
        if (reg == 0x10) printf(VFORMAT, "M4", "-HV",  (value * SINGLE_END * (-62.941)));
        if (reg == 0x12) printf(VFORMAT, "M3", "+HV",  (value * SINGLE_END * 62.941));
        if (reg == 0x14) printf(VFORMAT, "M2", "12V",  (value * SINGLE_END * 4.893));
        if (reg == 0x16) printf(VFORMAT, "M1", "5V",   (value * SINGLE_END * 2.0));
        if (reg == 0x18) printf(VFORMAT, "M7", "3V0A", (value * SINGLE_END * 1.401));
        if (reg == 0x1A) printf(TFORMAT, "T1", "TEMP", (value * 0.0625));
    }
}

int read_BITE() {

    int file;
    int error = 0;
    const char *filename = I2C_DEVICE;
    uint8_t init_data[2];

    usleep(1000000);// Wait 1 s
    printf("\033[2J\033[1;1H");

    // Open the I2C device
    if ((file = open(filename, O_RDWR)) < 0) {
        perror("Failed to open the I2C bus");
        return 1;
    }

    // Specify the address of the I2C slave device
    if (ioctl(file, I2C_SLAVE, LTC2991_I2C_ADDRESS_1) < 0) {
        perror("Failed to acquire bus access and/or talk to slave");
        return 1;
    }

    // Write to the LTC2991_CH_EN_TRIGGER register to enable all channels
    init_data[0] = LTC2991_CH_EN_TRIGGER;
    init_data[1] = LTC2991_V7_V8_EN | LTC2991_V5_V6_EN | LTC2991_V3_V4_EN | LTC2991_V1_V2_EN | LTC2991_T_INT_VCC_EN;

    if (write(file, init_data, 2) != 2) {
        perror("Failed to write to the I2C bus");
        return 1;
    }

    // Wait 2 ms
    usleep(2000);

    // Read and display all registers
    read_registers(file);
    
    // Close the I2C device
    close(file);

    // Compare read values to expected values 
    // TODO: Write code for comapring values
    if (error) {
        perror("Unexpected voltage measured");
        return 1;
    }

    return 0;
}
