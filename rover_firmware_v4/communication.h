// Library for implementing communication protocol between RAPID unit and motor controller board

#ifndef communication_h
#define communication_h

// Command constants
const int CMD_SIZE = 32;
const byte START_BYTE = 0x21; // '!' Start byte

const byte CMD_SET_SPEED = 0x73; // 's' : Set carriage speed to {} mm/s

const byte CMD_RESET_NEEDLE = 0x72; // 'r' : Reset needle position to zero
const byte CMD_NEEDLE_DOWN = 0x64; // 'd' : Move needle down by {} * 0.1 mm
const byte CMD_NEEDLE_UP = 0x75; // 'u' : Move needle up by {} * 0.1 mm
const byte CMD_MOVE_NEEDLE = 0x70; // 'p' : Move needle to position {} * 0.1 mm
const byte CMD_READ_NEEDLE = 0x6E; // 'n' : Read needle current position

const byte CMD_RESET_CARRIAGE = 0x7A; // 'z' : Reset carriage position to zero
const byte CMD_CARRIAGE_LEFT = 0x2D; // '-' : Move carriage to left by {} * 0.1 mm
const byte CMD_CARRIAGE_RIGHT = 0x2B; // '+' : Move carriage to right by {} * 0.1 mm
const byte CMD_MOVE_CARRIAGE = 0x6D; // 'm' : Move carriage to position {} * 0.1 mm
const byte CMD_READ_CARRIAGE = 0x67; // 'g' : Read carriage current position

const byte CMD_READ_TIME = 0x74; // 't' : Read current time
const byte CMD_READ_LOG = 0x4C; // 'L' : Read log

const byte END_BYTE = 0x23; // '#' End byte

// Read and unpack command message
byte read_cmd(){

  byte command = 0;
  int length = 0;
  byte payload[10] = {};

  if (Serial.available() > 0){
    char cmd[CMD_SIZE] = {};
    Serial.readBytes(cmd, sizeof(cmd));

    if (cmd[0] == START_BYTE) {
      command = cmd[1];
      length = (uint8_t)cmd[2];
      for (int i=0; i<length; i++){
        payload[i] = (uint8_t)cmd[3+i];
      }
    }
    else {
      Serial.println("Invalid START byte");
    }

  }
  return command;
}

void send_rsp(){

  String rsp = "";
  Serial.println(rsp);
}


#endif