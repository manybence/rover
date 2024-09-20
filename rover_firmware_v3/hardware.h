// Defining hardware configuration variables

#ifndef hardware_h
#define hardware_h

#include "UstepperS32.h"

#define XMAX 525
#define XMIN 0

UstepperS32 stepper;

// Used to keep track of configuration
struct
{
	float acceleration = 5000.0; // In steps/s
	float velocity = 1600.0;		 // In steps/s = 60 RPM
	uint8_t brake = COOLBRAKE;
	boolean closedLoop = false;
	float homeVelocity = 40.0; // In rpm
	int8_t homeThreshold = 15;
	bool homeDirection = CW; // In rpm
} conf;

//Carriage configuration
float angle = (360.0/8.0); //1 mmm movement (1mm)amount of degrees to move 8 mm pr revolution
const float A_TO_MM = (36.0 / 8.0) / 204.2692; //Absolute encoder to mm conversion factor (measured emperical value)
const float MIN_DIST_MM = 0.1;
const float MAX_DIST_MM = 84.0;
const int MAXN = 85100; 
float zero_pos = 0.0;
float a = 0.0;
int m = HAL_GetTick();
int logIndex = 0;

//Sensors
bool needle_switch = true;
bool left_switch = true;
bool right_switch = true;

//Needle configuration
//PWM frequency is 1 kHz
const float DEPTH_TO_TIME = 1000 * 200 / (0.5 * 1000);   // time / depth = Steps per revolution / Step mode (half) * PWM frequency. Pitch of shaft is 1 mm/rev 
const float MAX_DEPTH = 28;
const float MIN_DEPTH = 1;
float depth = MIN_DEPTH;
bool zeroed = false;
bool needleup = false;

//Define IO pins
int L_SWITCH_PIN = 11;  //Connect COM pin of switch to GND
int R_SWITCH_PIN = 10; //Connect COM pin of switch to GND
int N_SWITCH_PIN = 9; //Connect COM pin of switch to GND
int DIR_PIN = 7;
int STEP_PIN = 6;
int M3_PIN = 5;
int M2_PIN = 4;
int M1_PIN = 3;
int EN_PIN = 2;

void config_hardware(){

  // Configure IO pins
  pinMode(N_SWITCH_PIN, INPUT_PULLUP);
  pinMode(L_SWITCH_PIN, INPUT_PULLUP);
  pinMode(R_SWITCH_PIN, INPUT_PULLUP);
  pinMode(EN_PIN, OUTPUT);
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  pinMode(M1_PIN, OUTPUT);
  pinMode(M2_PIN, OUTPUT);
  pinMode(M3_PIN, OUTPUT);

  //Configure stepper motor
 	stepper.setup(CLOSEDLOOP, 200);
	stepper.disableClosedLoop();
	stepper.checkOrientation(30.0);       //Check orientation of motor connector with +/- 30 microsteps movement  
	stepper.setMaxAcceleration(conf.acceleration);
	stepper.setMaxDeceleration(conf.acceleration);
	stepper.setMaxVelocity(conf.velocity);
  stepper.setControlThreshold(15);
  stepper.setHoldCurrent(0.001);
  stepper.setCurrent(25);
  stepper.clearStall();

  // Configure needle stepper
  digitalWrite(M1_PIN, HIGH); // Half-step mode: M1=H, M2=L
  digitalWrite(M2_PIN, LOW);
  digitalWrite(M3_PIN, LOW);  // Configuration pin for internal voltage reference. Must be set to LOW
  analogWrite(STEP_PIN, 0);
  digitalWrite(EN_PIN, HIGH);
}

void start_needle(bool direction){
  digitalWrite(EN_PIN, LOW);
  digitalWrite(DIR_PIN, direction);
  analogWrite(STEP_PIN, 140);
}

void stop_needle(){
  analogWrite(STEP_PIN, 0);
  digitalWrite(EN_PIN, HIGH);
}

void needle_reset(){
  // Resetting needle to highest position
  const long timeout = 5000; 
  unsigned long startTime = millis();

  start_needle(true);

  // Add timeout limit
  while (digitalRead(N_SWITCH_PIN)){
    if (millis() - startTime > timeout) {
      Serial.println("Timeout error.");
      break;
    }
  }

  stop_needle();
  depth = MIN_DEPTH;
  needleup = true;
}

void moveToAngleAtVelocity(float angle) {
  Serial.println(angle/10.0);
  stepper.moveToAngle((zero_pos - angle) / A_TO_MM / 10.0);
}

void stopWatchInit() {
  CoreDebug->DEMCR |= 0x01000000; //Enable the use of DWT.
}

void stopWatchStart() {
  DWT->CYCCNT = 0; //Reset cycle counter.
  DWT->CTRL |= 0x1; //Enable cycle counter.
}

uint32_t stopWatchGet() {
  return DWT->CYCCNT; //Number of clock cycles stored in count variable.
}

void needle_to_pos(int depth_target) {
  needleup = false;
  if ((depth_target >= MIN_DEPTH) && (depth_target <= MAX_DEPTH)) {
    float d_t = abs(depth_target - depth) * DEPTH_TO_TIME;
    start_needle(depth_target < depth);
    delay(d_t);
    stop_needle();
    depth = depth_target;
  }
}

void move_needle(bool direction) {
  // Moving the needle up/down by 1 mm

  // Moving up
  if (direction) {
    if (digitalRead(N_SWITCH_PIN)) {
      Serial.println("Moving needle up by 1 mm");
      start_needle(true);
      delay(400); // 400 ms translates to 1 mm movement
      stop_needle();
      depth -= 1;
    }
  }

  // Moving down
  else {
    if (depth < MAX_DEPTH) {
      Serial.print("Moving needle down by 1 mm");
      start_needle(false);
      delay(400); // 400 ms translates to 1 mm movement
      stop_needle();
      depth += 1;
    }
  }
}

void reset_carriage() {
  stepper.clearStall();
  while (digitalRead(L_SWITCH_PIN) && digitalRead(R_SWITCH_PIN)) {
    stepper.moveAngle(-0.5 * angle); // larger (0.5mm) step back
  }
  while (!(digitalRead(L_SWITCH_PIN) && digitalRead(R_SWITCH_PIN))) {
    stepper.moveAngle(0.01 * angle); // smaller (0.01mm) step forward
  }
  delay(200);
  stepper.encoder.init();
  stepper.encoder.setHome(0.0);
  zero_pos = stepper.encoder.getAngleMoved();
  m = HAL_GetTick();
  logIndex = 0;
  zeroed = true;
  stopWatchStart();
}

void carriage_to_pos(int xpos) {
  if ((XMIN <= xpos) && (xpos <= XMAX) && zeroed)
    stepper.moveToAngle(xpos * (2270.82 / 500.0));
}

int read_carriage() {
  a = stepper.encoder.getAngleMoved();
  int i = round((a - zero_pos)* A_TO_MM * 10.0);
  return(i); 
}

void read_switches() {

  // Read needle microswitch
  if (!digitalRead(N_SWITCH_PIN)){
    depth = MIN_DEPTH;
    if (needle_switch)
      Serial.println("Needle switch activated");
    needle_switch = false;
  } 

  // Read left microswitch
  if (!digitalRead(L_SWITCH_PIN)){
    if (left_switch)
      Serial.println("Left switch activated");
    left_switch = false;
  } 

  // Read right microswitch
  if (!digitalRead(R_SWITCH_PIN)){
    if (right_switch)
      Serial.println("Right switch activated");
    right_switch = false;
  } 
}


#endif