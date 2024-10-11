#include "UstepperS32.h"
#include "hardware.h"
#include "communication.h"
#include "base64.h"

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

void update_log() {
  int currentXpos = round((stepper.encoder.getAngleMoved()- zero_pos) * ANGLE_TO_MM);
  if (abs(currentXpos - lastXpos) >= CONTROL_THRESHOLD) {
    logXpos(currentXpos);
    lastXpos = currentXpos;
  }
}

void readLog() {
  int t0 = logBuffer[0].time;
  for (int i = 0; i < logIndex; i++) {
    response += "X" + String(logBuffer[i].xpos) + "T" + String(logBuffer[i].time - t0) + "\n";
  }
}

void readLogCondensed() { // condensed format
  int td;
  char str[5];
  if (logIndex >= 1) {

    // Attach X positions
    response = "[" + String(logBuffer[0].xpos) + "," + String(logBuffer[logIndex-1].xpos) + "]";

    // Attach time difference
    for (int i = 1; i < logIndex; i++) {
      td = logBuffer[i].time-logBuffer[i-1].time;
      if (td > 4095) td = 4095;
      String base64str = String(Base64::b64ConvertInt(td, 2).c_str());
      base64str.toCharArray(str, 5);
      response += str;
    }

    // Attach end of line symbol
    response += "#\n";
  }
}

void execute_command(Command command) { 
  switch (command.cmd) {

    case CMD_RECONFIG: {
      config_hardware();
      response = "Hardware reconfigured.";
      break;
    }

    case CMD_SET_SPEED: {
      if (command.payload > MIN_SPEED) {
        stepper.setMaxVelocity(command.payload * MM_TO_STEP);
        response = "Velocity set to: " + String(command.payload, 2) + " mm/s";
      }
      else {
        response = "Velocity is too low";
      }
      break;
    }

    case CMD_RESET_NEEDLE: {
      needle_reset();
      response = "Needle resetted";
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
      needle_to_pos(command.payload);
      break;
    }
    
    case CMD_READ_NEEDLE: {
      response = "Reading needle pos: " + String(depth, 2);
      break;
    }

    case CMD_RESET_CARRIAGE: {
      reset_carriage();
      break;
    }

    case CMD_CARRIAGE_LEFT: {
      carriage_left(command.payload);
      break;
    }

    case CMD_CARRIAGE_RIGHT: {
      carriage_right(command.payload);
      break;
    }

    case CMD_MOVE_CARRIAGE: {
      carriage_to_pos(command.payload);
      break;
    }
      
    case CMD_READ_CARRIAGE: {
      response = "Reading carriage pos: " + String(read_carriage(), 2);
      break;
    }
      
    case CMD_READ_TIME: {
      response = "Reading time: " + String(stopWatchGet()/84000);
      break;
    }

    case CMD_READ_LOG: {
      readLog();
      break;
    }

    case CMD_READ_LOG_COND: {
      readLogCondensed();
      break;
    }

    default:
      response = "Unknown Command";
      break;
  }
}