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
  stepper.setCurrent(5.0); //Maybe increase
  stepper.clearStall();
  stepper.moveAngle(1);

  //Configure needle stepper
  digitalWrite(M1_PIN, HIGH);
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
    depth = 0.0;
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

  //Move needle to specific position. Enter command in format: p f.f (example p 12.5). The depth is in mm.
  if (cmd == 'p'){
    float depth_target = Serial.parseFloat();
    if ((depth_target >= 0.0) && (depth_target <= MAX_DEPTH)) {
      Serial.print("The target depth is: ");
      Serial.println(depth_target);
      float d_t = abs(depth_target - depth) * DEPTH_TO_TIME;
      start_needle(depth_target < depth);
      delay(d_t);
      stop_needle();
      depth = depth_target;
    }
  }

  //Reset needle position (topmost)
  else if(cmd == 'r'){
    needle_reset();
    depth = 0.0;
  }

  //Move needle down by 1 mm
  else if(cmd == 'd' && depth < MAX_DEPTH){
    start_needle(false);
    delay(400);
    stop_needle();
    depth += 1;
  }

  //Move needle up by 1 mm
  else if(cmd == 'u' && digitalRead(N_SWITCH_PIN)){
    start_needle(true);
    delay(400);
    stop_needle();
    depth -= 1;
  }

  //Move to the left
  else if(cmd == '-' && digitalRead(L_SWITCH_PIN)){
    stepper.moveAngle(angle);
  }
  
  //Move to the right
  else if(cmd == '+' && digitalRead(R_SWITCH_PIN)){
    stepper.moveAngle(-angle);
  }

  //Get current horizontal position
  else if(cmd == 'g'){
    a = stepper.encoder.getAngleMoved();
    Serial.println(zero - a * A_TO_MM); 
  }

  //Move to zero position on horizontal axis
  else if(cmd == 'z'){  //zero command               
    stepper.clearStall();
    while (digitalRead(L_SWITCH_PIN) && digitalRead(R_SWITCH_PIN)) {
      stepper.moveAngle(2.0*angle); //larger (0.5mm) step back
    }
  }
}