#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include "gpio_handler.hpp"

#define I2C_DEVICE "/dev/i2c-1"
#define LTC2991_I2C_ADDRESS_1 0x48
#define LTC2991_I2C_ADDRESS_2 0x49
#define SINGLE_END 0.00030518
#define VFORMAT "%-3s %5s: %8.2f V\n"
#define IFORMAT "%-3s %5s: %8.0f mA\n"
#define TFORMAT "%-3s %5s: %8.0f C\n"

#define LTC2991_CH_EN_TRIGGER 0x01
#define NUM_REGISTERS 0x1D // Number of registers to read

#define BIT(n) (1U << (n))

#define LTC2991_STATUS_LOW		0x00
#define LTC2991_CH_EN_TRIGGER		0x01
#define LTC2991_V1_V4_CTRL		0x06
#define LTC2991_V5_V8_CTRL		0x07
#define LTC2991_PWM_TH_LSB_T_INT	0x08
#define LTC2991_PWM_TH_MSB		0x09
#define LTC2991_CHANNEL_V_MSB(x)	(0x0A + ((x) * 2))
#define LTC2991_CHANNEL_T_MSB(x)	(0x0A + ((x) * 4))
#define LTC2991_CHANNEL_C_MSB(x)	(0x0C + ((x) * 4))
#define LTC2991_T_INT_MSB		0x1A
#define LTC2991_VCC_MSB			0x1C
#define LTC2991_V7_V8_EN		BIT(7)
#define LTC2991_V5_V6_EN		BIT(6)
#define LTC2991_V3_V4_EN		BIT(5)
#define LTC2991_V1_V2_EN		BIT(4)
#define LTC2991_T_INT_VCC_EN		BIT(3)
#define LTC2991_V3_V4_FILT_EN		BIT(7)
#define LTC2991_V3_V4_TEMP_EN		BIT(5)
#define LTC2991_V3_V4_DIFF_EN		BIT(4)
#define LTC2991_V1_V2_FILT_EN		BIT(3)
#define LTC2991_V1_V2_TEMP_EN		BIT(1)
#define LTC2991_V1_V2_DIFF_EN		BIT(0)
#define LTC2991_V7_V8_FILT_EN		BIT(7)
#define LTC2991_V7_V8_TEMP_EN		BIT(5)
#define LTC2991_V7_V8_DIFF_EN		BIT(4)
#define LTC2991_V5_V6_FILT_EN		BIT(7)
#define LTC2991_V5_V6_TEMP_EN		BIT(5)
#define LTC2991_V5_V6_DIFF_EN		BIT(4)
#define LTC2991_REPEAT_ACQ_EN		BIT(4)
#define LTC2991_T_INT_FILT_EN		BIT(3)
#define LTC2991_MAX_CHANNEL		4
#define LTC2991_T_INT_CH_NR		4
#define LTC2991_VCC_CH_NR		0

void read_registers_1(int file) {
    uint8_t buffer[3];
    uint8_t reg;
    uint16_t value;
    double V12 = 12.0;
    for (reg = 0x00; reg <= NUM_REGISTERS; reg++) {
        // Write the register address to the I2C bus
        if (write(file, &reg, 1) != 1) {
            perror("Failed to write to the i2c bus");
            return;
        }

        // Read data from the I2C bus
        if (read(file, buffer, 2) != 2) {
            perror("Failed to read from the i2c bus");
            return;
        }

        // Combine the two bytes read into a single 16-bit value
        value = (buffer[0] << 8) | buffer[1];

    };
    usleep(10000);
    for (reg = 0x00; reg <= NUM_REGISTERS; reg++) {
        // Write the register address to the I2C bus
        if (write(file, &reg, 1) != 1) {
            perror("Failed to write to the i2c bus");
            return;
        }

        // Read data from the I2C bus
        if (read(file, buffer, 2) != 2) {
            perror("Failed to read from the i2c bus");
            return;
        }

        // Combine the two bytes read into a single 16-bit value
        value = (buffer[0] << 8) | buffer[1];
        //printf("Register 0x%02X: %04X\n", reg, value);
        if (reg == 0x0A) printf(VFORMAT, "M10","5V0A", (value * SINGLE_END * 2.0));
        if (reg == 0x0C) printf(VFORMAT, "M6", "8V6",  (value * SINGLE_END * 3.495));
        if (reg == 0x0E) printf(VFORMAT, "M5", "3V3D", (value * SINGLE_END * 1.287));
        if (reg == 0x10) printf(VFORMAT, "M4", "-HV",  (value * 3.28986589386923 / 1000.0)-116.205697641691);

        if (reg == 0x12) printf(VFORMAT, "M3", "+HV",  (value * SINGLE_END * 62.941));
        if (reg == 0x14) {
            V12 = value * SINGLE_END * 4.893;
            printf(VFORMAT, "M2", "12V",  V12);
        }    
        
        if (reg == 0x16) printf(VFORMAT, "M1", "5V",   (value * SINGLE_END * 2.0));
        if (reg == 0x18) printf(VFORMAT, "M7", "3V0A", (value * SINGLE_END * 1.401));
        //if (reg == 0x1A) printf(TFORMAT, "T1", "TEMP", (value * 0.0625));

    }

    reg = 0x1A;
    if (write(file, &reg, 1) != 1) {
       perror("Failed to write to the i2c bus");
       return;
    }
    if (read(file, buffer, 2) != 2) {
       perror("Failed to read from the i2c bus");
       return;
    }

    // Combine the two bytes read into a single 16-bit value
    value = (buffer[0] << 8) | buffer[1];
     if (reg == 0x1A) printf(TFORMAT, "T1", "TEMP", (value * 0.0625));

    
}

void read_registers_2(int file) {
    uint8_t buffer[3];
    uint8_t reg;
    uint16_t value;

    for (reg = 0x00; reg <= NUM_REGISTERS; reg++) {
        // Write the register address to the I2C bus
        if (write(file, &reg, 1) != 1) {
            perror("Failed to write to the i2c bus");
            return;
        }

        // Read data from the I2C bus
        if (read(file, buffer, 2) != 2) {
            perror("Failed to read from the i2c bus");
            return;
        }

        // Combine the two bytes read into a single 16-bit value
        value = (buffer[0] << 8) | buffer[1];
    };
    usleep(10000);
    for (reg = 0x00; reg <= NUM_REGISTERS; reg++) {
        // Write the register address to the I2C bus
        if (write(file, &reg, 1) != 1) {
            perror("Failed to write to the i2c bus");
            return;
        }

        // Read data from the I2C bus
        if (read(file, buffer, 2) != 2) {
            perror("Failed to read from the i2c bus");
            return;
        }

        // Combine the two bytes read into a single 16-bit value
        value = (buffer[0] << 8) | buffer[1];
        if (reg == 0x0A) printf(VFORMAT, "M9","2V5", (value * SINGLE_END * 1.0));
        if (reg == 0x0C) printf(VFORMAT, "M8", "1V2",  (value * SINGLE_END * 1.0));
        if (reg == 0x14) printf(IFORMAT, "M14", "3V3I",  (value * 1.0));
        //if (reg == 0x1A) printf(TFORMAT, "T2", "TEMP", (value * 0.0625));

    }
    
}

int read_BITE() {

    int file;
    int error = 0;
    const char *filename = I2C_DEVICE;
    uint8_t init_data[8];

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
    init_data[2] = 0;
    init_data[3] = 0;
    init_data[4] = 0;
    init_data[5] = 0;
    init_data[6] = 0;
    init_data[7] = 0;

    if (write(file, init_data, 2) != 2) {
        perror("Failed to write to the I2C bus");
        return 1;
    }

    // Wait 2 ms
    usleep(2000);

    // Read and display all registers
    read_registers_1(file);
    
    // Close the I2C device
    close(file);
    
        // Open the I2C device
        if ((file = open(filename, O_RDWR)) < 0) {
            perror("Failed to open the i2c bus");
            return 1;
        }

        // Specify the address of the I2C slave device
        if (ioctl(file, I2C_SLAVE, LTC2991_I2C_ADDRESS_2) < 0) {
            perror("Failed to acquire bus access and/or talk to slave");
            return 1;
        }

        // Write to the LTC2991_CH_EN_TRIGGER register to enable all channels
        init_data[0] = LTC2991_CH_EN_TRIGGER;
        init_data[1] = LTC2991_V7_V8_EN | LTC2991_V5_V6_EN | LTC2991_V3_V4_EN | LTC2991_V1_V2_EN | LTC2991_T_INT_VCC_EN;
        init_data[2] = 0;
        init_data[3] = 0;
        init_data[4] = 0;
        init_data[5] = 0;
        init_data[6] = 0;
        init_data[7] = 1;

        if (write(file, init_data, 8) != 8) {
            perror("Failed to write to the i2c bus");
            return 1;
        }

        // Wait 2 ms
        usleep(2000);

        // Read and display all registers
        read_registers_2(file);
      
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
