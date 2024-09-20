#include "UstepperS32.h"
#include "hardware.h"
#include "communication.h"

struct LogEntry {
  int xpos;
  unsigned long time;
};

const int BUFFER_SIZE = 1024; // Adjust buffer size as needed
const int CONTROL_THRESHOLD = 1; // Define a threshold for angle comparison
bool bufferFull = false;
int lastXpos = 0.0;
float target = 0.0;
bool targetReached = true;
LogEntry logBuffer[BUFFER_SIZE];

void logXpos(int xpos) {
  logBuffer[logIndex].xpos =  xpos;
  logBuffer[logIndex].time = ((HAL_GetTick() - m) + 50) / 100;
  logIndex = (logIndex + 1) % BUFFER_SIZE;
  if (logIndex == 0) {
    bufferFull = true;
  }
}

void readLog() {
  if (bufferFull) {
    for (int i = logIndex; i < BUFFER_SIZE; i++) {
      Serial.print("X");
      Serial.print(logBuffer[i].xpos);
      Serial.print("T");
      Serial.println(logBuffer[i].time);
    }
  }
  for (int i = 0; i < logIndex; i++) {
    Serial.print("X");
    Serial.print(logBuffer[i].xpos);
    Serial.print("T");
    Serial.println(logBuffer[i].time);
  }
}

void execute_command(Command command) {
  switch (command.cmd) {

    case CMD_SET_SPEED: {
      Serial.println("Setting speed to: ");
      stepper.setMaxVelocity(command.payload[0] * 25.433);
      break;
    }

    case CMD_RESET_NEEDLE: {
      Serial.println("Resetting needle pos");
      needle_reset();
      break;
    }

    case CMD_NEEDLE_DOWN: {
      move_needle(false);
      break;
    }

    case CMD_NEEDLE_UP: {
      move_needle(true);
      break;
    }

    case CMD_MOVE_NEEDLE: {
      Serial.println("Moving needle to pos: ");
      needle_to_pos(command.payload[0]);
      break;
    }
    
    case CMD_READ_NEEDLE: {
      Serial.print("Reading needle pos: ");
      Serial.println(depth); 
      break;
    }

    case CMD_RESET_CARRIAGE: {
      Serial.println("Resetting carriage pos");
      reset_carriage();
      break;
    }

    case CMD_CARRIAGE_LEFT: {
      if (digitalRead(L_SWITCH_PIN) && zeroed) {
        Serial.println("Moving carriage left by: ");
        stepper.moveAngle(-command.payload[0]);
      }
      break;
    }

    case CMD_CARRIAGE_RIGHT: {
      if (zeroed && digitalRead(R_SWITCH_PIN)){
        Serial.println("Moving carriage right by: ");
        stepper.moveAngle(command.payload[0]);
      }
      break;
    }

    case CMD_MOVE_CARRIAGE: {
      Serial.println("Moving carriage to pos: ");
      carriage_to_pos(command.payload[0]);
      break;
    }
      
    case CMD_READ_CARRIAGE: {
      Serial.print("Reading carriage pos: ");
      Serial.println(read_carriage());
      break;
    }
      
    case CMD_READ_TIME: {
      Serial.println("Reading time: ");
      Serial.println(HAL_GetTick() - m);
      Serial.println(stopWatchGet());
      break;
    }

    case CMD_READ_LOG: {
      Serial.println("Reading log: ");
      readLog();
      break;
    }

    default:
      Serial.println("Unknown Command");
      break;
  }
}