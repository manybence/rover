
#include <UstepperS32.h>

UstepperS32 stepper;
float angle = 0.1*(360.0/8.0); //0.1 mmm movement (1mm)amount of degrees to move 8 mm pr revolution
const float A_TO_MM = (36.0 / 8.0) / 204.2692; //Absolute encoder to mm conversion factor (measured emperical value)
const float MIN_DIST_MM = 0.1;
const float MAX_DIST_MM = 84.0;
const int MAXN = 85100; 
float zero = 0.0;
float a = 0.0;
int inPin = 8;
int val = 0; 
int n = 0; 
void setup() {
	stepper.setup(CLOSEDLOOP, 200);
	stepper.setMaxAcceleration(2000);
	stepper.setMaxVelocity(800);
	stepper.checkOrientation(30.0);
	//stepper.setControlThreshold(15);
  stepper.setCurrent(10.0);//use software current setting

	//delay(1000);
  
 //stepper.clearStall();
 Serial.begin(115200);
/*
 while ((digitalRead(inPin)) && (n < MAXN)) {
    stepper.moveAngle(5.0 * angle);
    n = n + 1;
	  a = stepper.encoder.getAngleMoved();
    Serial.println("READY");
  };

  while ((!digitalRead(inPin)) && (n < MAXN)) {
    stepper.moveAngle(-angle/10.0);
    n = n - 1;
  }
  zero = a * A_TO_MM;
  a = stepper.encoder.getAngleMoved();
  n = 0;
  */
}

void loop() {
  char cmd;
  while(!Serial.available());
  cmd = Serial.read();
  if(cmd == '-')                      
  {
	  a = stepper.encoder.getAngleMoved();
  	if ((zero - a * A_TO_MM) > MIN_DIST_MM)
       stepper.moveAngle(angle);
  }
  
  else if(cmd == '+')                 
  {
	  a = stepper.encoder.getAngleMoved();
  	if ((zero - a * A_TO_MM) < MAX_DIST_MM)
       stepper.moveAngle(-angle);
  }
  else if(cmd == 'p')                 
  {
	  a = stepper.encoder.getAngleMoved();
	  Serial.println(zero - a * A_TO_MM); 
  }
  else if(cmd == 's')                 
  {
	  val = digitalRead(inPin);
    Serial.println(val);
  }

  else if(cmd == 'z')  //zero command               
  {
      stepper.clearStall();
      while ((digitalRead(inPin)) && (n < MAXN)) {
  	    stepper.moveAngle(2.0*angle); //larger (0.5mm) step back
        n = n + 1;
      };

      while ((!digitalRead(inPin)) && (n < MAXN)) {
  	    stepper.moveAngle(-angle/10.0); //small 0.01 mm  step forward
        n = n - 1;
      };
      a = stepper.encoder.getAngleMoved();
      zero =  a * A_TO_MM;
       Serial.flush();
      Serial.println("READY");
      stepper.stop();
     n = 0;
  }
  else
  {
  };
}