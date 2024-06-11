#include "rover_lib.h"

void setup() {
	// stepper.setup(CLOSEDLOOP, 200);
	// stepper.setMaxAcceleration(2000);
	// stepper.setMaxVelocity(800);
	// stepper.checkOrientation(30.0);
  // stepper.setCurrent(10.0);//use software current setting
  Serial.begin(9600);

  pinMode(L_SWITCH_PIN, INPUT_PULLUP);
  pinMode(R_SWITCH_PIN, INPUT_PULLUP);
  pinMode(EN_PIN, OUTPUT);
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  pinMode(M1_PIN, OUTPUT);
  pinMode(M2_PIN, OUTPUT);

  //Set full step mode
  digitalWrite(M1_PIN, LOW);
  digitalWrite(M2_PIN, LOW);
}


void loop() {

  char cmd;

  //Read microswitches
  if (!digitalRead(L_SWITCH_PIN)){
    Serial.println("Left switch activated");
  } 
  if (!digitalRead(R_SWITCH_PIN)){
    Serial.println("Right switch activated");
  } 
  delay(200);

  while(!Serial.available());
  cmd = Serial.read();

  //Move needle down
  if(cmd == 'x'){
    start_needle(true);
    delay(500);
    stop_needle();
  }

  //Move needle up
  if(cmd == 'y'){
    start_needle(false);
    delay(500);
    stop_needle();
  }

  /*
  if(cmd == '-'){
    stepper.moveAngle(angle);
  }
  
  else if(cmd == '+'){
    stepper.moveAngle(-angle);
  }

  else if(cmd == 'p'){
	  a = stepper.encoder.getAngleMoved();
	  Serial.println(zero - a * A_TO_MM); 
  }

  else if(cmd == 'z')  //zero command               
  {
    stepper.clearStall();
    while (!digitalRead(LeftSwitchPin) or !digitalRead(RightSwitchPin)) {
      stepper.moveAngle(2.0*angle); //larger (0.5mm) step back
    }
  }
  */
}