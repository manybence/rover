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

  // Timing the loop
  delay(50);

  // Read and execute incoming command
  Command command = read_cmd();
  if (command.cmd) {
    execute_command(command);
  } 

  // Check for angle changes and log them
  update_log();

  // Send response back to master
  send_response();
}

