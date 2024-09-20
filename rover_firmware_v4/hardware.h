// Defining hardware configuration variables

#ifndef communication_h
#define communication_h

#define XMAX 525
#define XMIN 0

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

float angle = (360.0/8.0); //1 mmm movement (1mm)amount of degrees to move 8 mm pr revolution
const float A_TO_MM = (36.0 / 8.0) / 204.2692; //Absolute encoder to mm conversion factor (measured emperical value)
const float MIN_DIST_MM = 0.1;
const float MAX_DIST_MM = 84.0;
const int MAXN = 85100; 
float zero_pos = 0.0;
float a = 0.0;

//Needle seettings
//PWM frequency is 1 kHz
const float DEPTH_TO_TIME = 1000 * 200 / (0.5 * 1000);   // time / depth = Steps per revolution / Step mode (half) * PWM frequency. Pitch of shaft is 1 mm/rev 
const float MAX_DEPTH = 28;
const float MIN_DEPTH = 1;
float depth = MIN_DEPTH;

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

  // Configure needle stepper
  digitalWrite(M1_PIN, HIGH); // Half-step mode: M1=H, M2=L
  digitalWrite(M2_PIN, LOW);
  digitalWrite(M3_PIN, LOW);  // Configuration pin for internal voltage reference. Must be set to LOW
  analogWrite(STEP_PIN, 0);
  digitalWrite(EN_PIN, HIGH);
}

#endif