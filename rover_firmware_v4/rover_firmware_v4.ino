#include "communication.h"
#include "hardware.h"

void setup() {
  Serial.begin(115200);
}

void loop() {

  byte command = read_cmd();

  // Handle command
  if (command) {
    switch (command) {

      case CMD_SET_SPEED:
        Serial.println("Setting speed");
        break;

      case CMD_RESET_NEEDLE:
        Serial.println("Resetting needle pos");
        break;

      case CMD_NEEDLE_DOWN:
        Serial.println("Moving needle down by");
        break;

      case CMD_NEEDLE_UP:
        Serial.println("Moving needle up by");
        break;

      case CMD_MOVE_NEEDLE:
        Serial.println("Moving needle to pos");
        break;
    
      case CMD_READ_NEEDLE:
        Serial.println("Reading needle pos");
        break;

      case CMD_RESET_CARRIAGE:
        Serial.println("Resetting carriage pos");
        break;

      case CMD_CARRIAGE_LEFT:
        Serial.println("Moving carriage left by");
        break;

      case CMD_CARRIAGE_RIGHT:
        Serial.println("Moving carriage right by");
        break;

      case CMD_MOVE_CARRIAGE:
        Serial.println("Moving carriage to pos");
        break;

      case CMD_READ_CARRIAGE:
        Serial.println("Reading carriage pos");
        break;

      case CMD_READ_TIME:
        Serial.println("Reading time");
        break;

      case CMD_READ_LOG:
        Serial.println("Reading log");
        break;

      default:
        Serial.println("Unknown Command");
        break;
    }
  }
  

  delay(500);

}

