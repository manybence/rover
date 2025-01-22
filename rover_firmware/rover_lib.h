#include "UstepperS32.h"
#include "hardware.h"
#include "communication.h"
#include "base64.h"

//Store version number via CRC
#define FLASH_START_ADDRESS 0x08000000  // Start of flash memory
#define FLASH_SIZE          0x20000     // Size of flash memory to include in CRC (e.g., 128 KB)
#define CRC32_POLYNOMIAL    0xEDB88320  // CRC-32 polynomial
const String compilationDate = String(__DATE__) + " " + String(__TIME__);
const String revNum = "1.0.0";

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
  if (logIndex >= BUFFER_SIZE) {
    bufferFull = true;
    return;
  }  
  logBuffer[logIndex].xpos =  xpos;
  logBuffer[logIndex].time = stopWatchGet()/84000;
  logIndex += 1;
}

uint32_t calculate_flash_crc() {
  uint32_t crc = 0xFFFFFFFF;  // Initial CRC value
  uint32_t *flash_ptr = (uint32_t *)FLASH_START_ADDRESS;  // Pointer to flash memory
  uint32_t flash_words = FLASH_SIZE / 4;  // Number of 32-bit words in flash

  for (uint32_t i = 0; i < flash_words; i++) {
    uint32_t word = *flash_ptr++;  // Read 32-bit word from flash memory
    crc ^= word;  // XOR with the current CRC

    // Process each bit
    for (uint8_t j = 0; j < 32; j++) {
      if (crc & 1) {
        crc = (crc >> 1) ^ CRC32_POLYNOMIAL;
      } else {
        crc >>= 1;
      }
    }
  }

  return ~crc; 
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
  else response = "[0,0]0#\n";
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

    case CMD_READ_ZEROED: {
      response = String(zeroed);
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
      response = String(read_carriage(), 2);
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

    case CMD_READ_VERSION:  {
      // response = String(calculate_flash_crc());  // Return CRC of firmware as version number
      response = revNum;
      break;
    }

    case CMD_READ_DATE: {
      response = compilationDate;
      break;
    }

    default:
      response = "Unknown Command";
      break;
  }
}

