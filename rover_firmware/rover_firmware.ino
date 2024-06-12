#include "rover_lib.h"

void setup() {

  //Configure pins
  pinMode(N_SWITCH_PIN, INPUT_PULLUP);
  pinMode(L_SWITCH_PIN, INPUT_PULLUP);
  pinMode(R_SWITCH_PIN, INPUT_PULLUP);
  pinMode(EN_PIN, OUTPUT);
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  pinMode(M1_PIN, OUTPUT);
  pinMode(M2_PIN, OUTPUT);

  //Configure horizontal stepper
	stepper.setup(CLOSEDLOOP, 200);
	stepper.setMaxAcceleration(2000);
	stepper.setMaxVelocity(800);
  stepper.setControlThreshold(15);
  stepper.setHoldCurrent(5);
  stepper.setCurrent(15.0); //Maybe increase
  stepper.clearStall();

  //Configure needle stepper
  digitalWrite(M1_PIN, LOW);
  digitalWrite(M2_PIN, LOW);
  analogWrite(STEP_PIN, 0);
  digitalWrite(EN_PIN, HIGH);

  Serial.begin(9600);
}


void loop() {

  char cmd;

  //Read microswitches
  if (!digitalRead(N_SWITCH_PIN)){
    Serial.println("Needle switch activated");
  } 
  if (!digitalRead(L_SWITCH_PIN)){
    Serial.println("Left switch activated");
  } 
  if (!digitalRead(R_SWITCH_PIN)){
    Serial.println("Right switch activated");
  } 
  delay(200);

  //while(!Serial.available());
  cmd = Serial.read();


  //Move needle down
  if(cmd == 'r'){
    needle_reset();
  }

  //Move needle down
  if(cmd == 'd'){
    start_needle(true);
    delay(500);
    stop_needle();
  }

  //Move needle up
  if(cmd == 'u' && digitalRead(N_SWITCH_PIN)){
    start_needle(false);
    delay(500);
    stop_needle();
  }

  //TODO: Add uswitch limits!
  if(cmd == '-' && digitalRead(L_SWITCH_PIN)){
    stepper.moveAngle(angle);
  }
  
  else if(cmd == '+' && digitalRead(R_SWITCH_PIN)){
    stepper.moveAngle(-angle);
  }

  else if(cmd == 'p'){
	  a = stepper.encoder.getAngleMoved();
	  Serial.println(zero - a * A_TO_MM); 
  }

  else if(cmd == 'z')  //zero command               
  {
    stepper.clearStall();
    while (digitalRead(L_SWITCH_PIN) && digitalRead(R_SWITCH_PIN)) {
      stepper.moveAngle(2.0*angle); //larger (0.5mm) step back
    }
  }
}