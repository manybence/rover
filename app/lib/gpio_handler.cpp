#include "gpio_handler.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <gpiod.h>
#include <cstdlib> // For system()
#include <string>

static struct gpiod_chip *chip = NULL;
static struct gpiod_line *lines[28];  // Adjust array size based on the number of GPIOs
int gpio_idx;

void executeShellCommand(const std::string& command) {
    int ret = system(command.c_str());
    if (ret != 0) {
        perror(("Failed to execute command: " + command).c_str());
    }
}

void MB_OFF()
{
  executeShellCommand("pinctrl set 4-26 no");
}

void Investigate_Flash_Configuration()
{
  executeShellCommand("pinctrl 4-26 no");
  executeShellCommand("sudo dtparam spi=off");
  executeShellCommand("pinctrl 4-26 no");
  executeShellCommand("pinctrl set 25 op dl");
  executeShellCommand("pinctrl set 14,15 op dh");
  executeShellCommand("pinctrl set 7-11 a0");
  executeShellCommand("sudo dtparam spi=on");
  executeShellCommand("sleep 0.1");
}

void Start_normal_operation()
{
  executeShellCommand("pinctrl 4-26 no");
  executeShellCommand("sudo dtparam spi=off");
  executeShellCommand("pinctrl set 7-11 ip pu");
  executeShellCommand("sleep 1");
  executeShellCommand("pinctrl set 14,15 op dh"); //3V3 and 5V on
  executeShellCommand("sleep 0.5");
  executeShellCommand("pinctrl set 25 op dl");
  executeShellCommand("sleep 0.5");
  executeShellCommand("pinctrl set 25 ip pu");    //Release FPGA 
  executeShellCommand("sleep 0.5");
  executeShellCommand("pinctrl set 7-11 a0");     //enable SPI
  executeShellCommand("sudo dtparam spi=on");
}

int initializeGPIO() {
    // Open the GPIO chip
    chip = gpiod_chip_open_by_name("gpiochip4");
    if (chip == NULL) {
        chip = gpiod_chip_open_by_name("gpiochip0");
        if (chip == NULL) {
            perror("Error opening GPIO chip");
            return -1;
        }
    }
    
    // Initialize each line with appropriate modes
    for (gpio_idx = 0; gpio_idx < 28; gpio_idx++)
      lines[gpio_idx] = 0;
    
    lines[IO_FILSEL] = gpiod_chip_get_line(chip, IO_FILSEL);
    if(lines[IO_FILSEL] == NULL) { printf("unable to open specific IO_FILSEL\n");}
    lines[IO_RST]    = gpiod_chip_get_line(chip, IO_RST);
    if(lines[IO_RST] == NULL) { printf("unable to open specific IO_RST\n");}
    lines[IO_HILO]   = gpiod_chip_get_line(chip, IO_HILO);
    if(lines[IO_HILO] == NULL) { printf("unable to open specific IO_HILO\n");}
    lines[IO_DONE]   = gpiod_chip_get_line(chip, IO_DONE);
    if(lines[IO_DONE] == NULL) { printf("unable to open specific IO_DONE\n");}
    lines[IO_RESET]  = gpiod_chip_get_line(chip, IO_RESET);
    if(lines[IO_RESET] == NULL) { printf("unable to open specific IO_RESET\n");}
//    lines[IO_F_CS]   = gpiod_chip_get_line(chip, IO_F_CS);
//    if(lines[IO_F_CS] == NULL) { printf("unable to open specific IO_F_CS\n");}
    lines[IO_EN3V3]  = gpiod_chip_get_line(chip, IO_EN3V3);
    if(lines[IO_EN3V3] == NULL) { printf("unable to open specific IO_EN3V3\n");}
    lines[IO_EN5V]   = gpiod_chip_get_line(chip, IO_EN5V);
    if(lines[IO_EN5V] == NULL) { printf("unable to open specific IO_EN5V\n");}
    lines[IO_P1]     = gpiod_chip_get_line(chip, IO_P1);
    if(lines[IO_P1] == NULL) { printf("unable to open specific IO_P1\n");}
    lines[IO_P2]     = gpiod_chip_get_line(chip, IO_P2);
    if(lines[IO_P2] == NULL) { printf("unable to open specific IO_P2\n");}
    lines[IO_P3]     = gpiod_chip_get_line(chip, IO_P3);
    if(lines[IO_P3] == NULL) { printf("unable to open specific IO_P3\n");}
    lines[IO_P4]     = gpiod_chip_get_line(chip, IO_P4);
    if(lines[IO_P4] == NULL) { printf("unable to open specific IO_P4\n");}

    //lines[IO_MOSI]   = gpiod_chip_get_line(chip, IO_MOSI);
    //if(lines[IO_MOSI] == NULL) { printf("unable to open specific IO_MOSI\n");}
    //lines[IO_MISO]   = gpiod_chip_get_line(chip, IO_MISO);
    //if(lines[IO_MISO] == NULL) { printf("unable to open specific IO_MISO\n");}
    //lines[IO_SCLK]   = gpiod_chip_get_line(chip, IO_SCLK);
    //if(lines[IO_SCLK] == NULL) { printf("unable to open specific IO_SCLK\n");}
    //lines[IO_ICE_CS] = gpiod_chip_get_line(chip, IO_ICE_CS);
    //if(lines[IO_ICE_CS] == NULL) { printf("unable to open specific IO_ICE_CS\n");}

    // Configure each line's direction
    gpiod_line_request_output(lines[IO_FILSEL], "RAPID", 0);
    gpiod_line_request_output(lines[IO_RST],    "RAPID", 0);
    gpiod_line_request_output(lines[IO_RESET],  "RAPID", 1);
    gpiod_line_request_output(lines[IO_HILO],   "RAPID", 0);
    gpiod_line_request_output(lines[IO_EN3V3],  "RAPID", 1);  // Initially on
    gpiod_line_request_output(lines[IO_EN5V],   "RAPID", 1);  // Initially on
    gpiod_line_request_output(lines[IO_P1],     "RAPID", 0);
    gpiod_line_request_output(lines[IO_P2],     "RAPID", 0);
    gpiod_line_request_output(lines[IO_P3],     "RAPID", 0);
    gpiod_line_request_output(lines[IO_P4],     "RAPID", 0);
    
    //gpiod_line_request_output(lines[IO_MOSI],   "RAPID", 0);
    //gpiod_line_request_output(lines[IO_SCLK],   "RAPID", 0);
    //gpiod_line_request_output(lines[IO_ICE_CS], "RAPID", 1);
    //gpiod_line_request_output(lines[IO_F_CS],   "RAPID", 1);

//    gpiod_line_request_input(lines[IO_RESET],  "RAPID");
    gpiod_line_request_input(lines[IO_DONE],    "RAPID");
    //gpiod_line_request_input(lines[IO_MISO],    "RAPID");

    return 0;
}

void setGPIOValue(int line_number, int value) {
   gpiod_line_set_value(lines[line_number], value);
}

int getGPIOValue(int line_number) {
    return gpiod_line_get_value(lines[line_number]);
}
void Release_RESET()
{
  //  gpiod_line_release(lines[IO_RESET]);
}
void cleanupGPIO() {
    // Release each line
    for (int gpio_idx = 0; gpio_idx < 28; ++gpio_idx) {
        if (lines[gpio_idx] != 0) {
            gpiod_line_release(lines[gpio_idx]);
        }
    }
    if (chip) {
        gpiod_chip_close(chip);
        chip = NULL;
    }
}
