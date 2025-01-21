// Library for implementing communication protocol between RAPID unit and motor controller board

#ifndef communication_h
#define communication_h

// Command constants
const int PAYLOAD_SIZE = 20;
const int CMD_SIZE = PAYLOAD_SIZE + 3;
const byte START_BYTE = 0x21; // '!' Start byte
const byte CMD_RECONFIG = 0x63; // 'c' : Reconfiguration of hardware

const byte CMD_RESET_NEEDLE = 0x72; // 'r' : Reset needle position to zero
const byte CMD_NEEDLE_DOWN = 0x64; // 'd' : Move needle down by 1 mm
const byte CMD_NEEDLE_UP = 0x75; // 'u' : Move needle up by 1 mm
const byte CMD_MOVE_NEEDLE = 0x70; // 'p' : Move needle to position {} mm
const byte CMD_READ_NEEDLE = 0x6E; // 'n' : Read needle current position

const byte CMD_SET_SPEED = 0x73; // 's' : Set carriage speed to {} mm/s
const byte CMD_RESET_CARRIAGE = 0x7A; // 'z' : Reset carriage position to zero
const byte CMD_CARRIAGE_LEFT = 0x2D; // '-' : Move carriage to left by {} mm
const byte CMD_CARRIAGE_RIGHT = 0x2B; // '+' : Move carriage to right by {} mm
const byte CMD_MOVE_CARRIAGE = 0x6D; // 'm' : Move carriage to position {} mm
const byte CMD_READ_CARRIAGE = 0x67; // 'g' : Read carriage current position
const byte CMD_READ_ZEROED = 0x6F; // 'o' : Read if carriage is zeroed (bool)

const byte CMD_READ_TIME = 0x74; // 't' : Read current time
const byte CMD_READ_LOG = 0x4C; // 'L' : Read log
const byte CMD_READ_LOG_COND = 0x6C; // 'l': Read log condensed format
const byte CMD_READ_VERSION = 0x76; // 'v' : Read version number (CRC)
const byte CMD_READ_DATE = 0x44; // 'D' : Read compilation date

String response = "";

struct Command {
  byte cmd;
  float payload = 0.0;
};

// Read and unpack command message
Command read_cmd(){

  Command command = {}; 

  if (Serial.available() > 0){
    char buffer[CMD_SIZE] = {};

    // Read incoming bytes
    Serial.readBytes(buffer, sizeof(buffer));

    // Check for START byte
    if (buffer[0] == START_BYTE) {

      // Read command
      command.cmd = buffer[1];

      // Read payload length
      int length = ((int)buffer[2] <= PAYLOAD_SIZE) ? (int)buffer[2] : PAYLOAD_SIZE;

      // Read payload
      byte payload_bytes[length];
      for (int i=0; i<length; i++){
        payload_bytes[i] = buffer[3+i];
      }

      // Convert the byte array to a float
      memcpy(&command.payload, payload_bytes, sizeof(command.payload));
    }
    else {
      Serial.println("Invalid START byte");
    }
  }
  response = "";
  return command;
}

// Send response to master
void send_response(){
  if (response != "")
    Serial.println(response);

}


#endif