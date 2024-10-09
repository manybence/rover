RAPID electronics unit - Instructions for use

-----------------------------------------------------------------------------------

Ports and connectors
X1-4: Ultrasound probe connectors. They are equivalent, only different connector types.
X5: 6V power input.
X6: Power output for motor driver. Not used at the moment.
X7: Communication port. Not used. 
X8, X9: USB ports for motor driver communication. They are equivalent. 
X10: Ethernet port for PC communication.

-----------------------------------------------------------------------------------

Setup
- Connect 6V power supply to port X5.
- Connect motor driver board to port X9 via USB-C cable. 
- Connect PC to port X10 via Ethernet cable.
- Connect A-mode and Doppler probes to any of X1-X4. Remember to select the correct port on the GUI before scanning.

The firmware (and different processing scripts) of the Raspberry Pi are located at:
home/rapid/projects/app

1. Open RealVNC viewer, locate rapidunit and connect. It might take a while for the Raspberry Pi to boot after being powered on.
2. Enter the IP address: rapidunit3.local
3. Enter username: rapid; password: 5353
4. Open Programming/Thonny to access the Python code for the web GUI. 
5. Run the script to start the GUI.
6. Open the browser and search for rapidunit[number]:8080 to access the GUI.

-----------------------------------------------------------------------------------
In case you want to modify the firmware that is responsible for the ultrasound scanning process and drives the motors:

Open Programming/Geany to access the C code

-----------------------------------------------------------------------------------
In case the motor driver unit needs to be reprogrammed, follow these steps:

To program the uStepper board:
1. Press and hold "BOOT"
2. Press and release "RESET"
3. Release "BOOT"

4. Open STM32CubeProgrammer software
5. Select USB device, connect
6. Mass erase internal memory

7. Repeat 1.-3.
8. Upload firmware from Arduino IDE

For further instructions on installing the libraries and STM32CubeProgrammer, see the official repository:
https://github.com/uStepper/uStepperS32