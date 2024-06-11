#include <UstepperS32.h>

UstepperS32 stepper;
float angle = 0.1*(360.0/8.0); //0.1 mmm movement (1mm)amount of degrees to move 8 mm pr revolution
const float A_TO_MM = (36.0 / 8.0) / 204.2692; //Absolute encoder to mm conversion factor (measured emperical value)
const float MIN_DIST_MM = 0.1;
const float MAX_DIST_MM = 84.0;
const int MAXN = 85100; 
float zero = 0.0;
float a = 0.0;
const long needle_pulse = 184;

//Define IO pins
int L_SWITCH_PIN = 8;  //Connect COM pin of switch to GND
int R_SWITCH_PIN = 7; //Connect COM pin of switch to GND
int M2_PIN = 5;
int M1_PIN = 4;
int EN_PIN = 3;
int STEP_PIN = 2;
int DIR_PIN = 1;



bool leftSwitch = false;
bool rightSwitch = false;

void start_needle(bool direction){
  digitalWrite(EN_PIN, LOW);
  digitalWrite(DIR_PIN, direction);
  analogWrite(STEP_PIN, 140);
}

void stop_needle(){
  analogWrite(STEP_PIN, 0);
  digitalWrite(EN_PIN, HIGH);
}