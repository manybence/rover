#include "communication.h"


void setup() {
  Serial.begin(115200);
}

void loop() {


  //Read and execute incoming command
  Command command = read_cmd();


}

