#include "rover_lib.h"
#include "communication.h"
#include "UstepperS32.h"
#include "hardware.h"

void setup() {
  stopWatchInit();
  config_hardware();
  Serial.begin(115200);
}

void loop() {

  // Read microswitches
  read_switches();
  delay(10);

  //Read and execute incoming command
  Command command = read_cmd();
  if (command.cmd) execute_command(command);

  // Check for angle changes and log them
  int currentXpos = round((stepper.encoder.getAngleMoved()- zero_pos) * A_TO_MM * 10.0);
  if (abs(currentXpos - lastXpos) >= CONTROL_THRESHOLD) {
    logXpos(currentXpos);
    lastXpos = currentXpos;
  }
}

