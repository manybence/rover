/*
 * Control software for pulsatile flow pummp. FL86STH118 motor + JT552 driver + Arduino Uno
 * author: Marcus Leander Jensen, Neurescue
 * date: 06/03/2023
 */

//---------------- Delays --------------
const int moveTime = 450;

//------------------- Set BPM -----------------------

//This parameter can be used to set an APPROXIMATE value 
//of the bpm of the pump. 
//Read the calculated values in the serial monitor

float bpm2 = 60; //Range: 0 < bpm2 < 66.6 (when the driver is set to 3200 pulses/rev)

const int exTime = moveTime+460;// The execution time of each loop
float pauseTop = (60000)/(bpm2)-exTime; //60 * 1000 ms/min 


//------------ Stepper motor pins ------------ 
const int pulse = 11;
const int dir = 13;
const int enable = 12;

//------------ Switch pins and variables ------------
const int toggle_motor = 7;
const int MS_reset = 9;
bool goingTop = true;

//-------------- BPM variables -----------------
float lastTime = 0;
int stepCount = 0;
float pulseTimer = 0;
int pulseCount = 0;
float pulseTimerSeconds = 0;
float bpm = 0;



void setup() {
  Serial.begin(9600);
  TCCR2B = TCCR2B & B11111000 | B00000010; // Set PWM frequency to 3.9 kHz
  
  /* 
   * The given frequency was calculated according to the following settings:
   * Pulse/rev = 1000 --> SW5/SW6/SW7/SW8 = on/on/on/off
   * Current = max --> SW1/SW2/SW3 = off/off/off
   * Half current (SW4) = off
   */

  
  pinMode(dir, OUTPUT);
  pinMode(enable, OUTPUT);
  digitalWrite(dir, HIGH); 
  analogWrite(pulse, 100);  // Set the outgoing pulse. 100/255 = 39% duty cycle (doesn't affect the motor's revolution)
  pinMode(toggle_motor, INPUT_PULLUP);
  pinMode(MS_reset, INPUT_PULLUP);
}

void loop() {

/*
Serial.print("toggle_motor:    ");
Serial.print(digitalRead(toggle_motor));
Serial.print("    |    ");
Serial.print("goingTop: ");
Serial.print(goingTop);
Serial.print("    |    ");
Serial.print("MS_reset: ");
Serial.println(digitalRead(MS_reset));

*/
//Serial.print(digitalRead(toggle_motor));

//If the microswitch for on/off is pushed an equal number of times
if (digitalRead(toggle_motor) == LOW & digitalRead(MS_reset) == LOW & goingTop == true) {
    digitalWrite(enable, LOW); //Disabling the motor 
    bpm = 0;
    pulseTimerSeconds = 0;
    //Serial.println(goingTop);
  }

 //If the microswitch for on/off is pushed an UNequal number of times
    else{

//and if the microswitch in the top of the cycle is pushed
      if (digitalRead(MS_reset) == LOW) {

//============== Counting BPM (RR-interval) =====================
pulseCount++;

        if (pulseCount > 1) { //RR-interval can be calculated with at lest 2 pulses
          pulseTimer = millis() - lastTime; //Time between a pulse and the previous pulse
          pulseTimerSeconds = (pulseTimer/1000); //
          bpm = 60/pulseTimerSeconds;
        }

lastTime = millis();

//================== Displaying BPM-variables =======================

//The RR-term refers two following R-peaks in an EKG which in short
//means that e.g. the RR-time is the time between two beats/pulses


Serial.print("RR-time (ms): ");
Serial.print(pulseTimer);
Serial.print("    |    ");
Serial.print("Top delay (ms): ");
Serial.print(pauseTop);
Serial.print("    |    ");
Serial.print("RR-BPM: ");
Serial.println(bpm);



//=================== Running the pump =========================

        digitalWrite(enable, LOW);    //Stop the motor
        delay(pauseTop);
        goingTop = false;
        digitalWrite(enable, HIGH); //Enable the motor and
        digitalWrite(dir, HIGH); //change movement direction        
        delay(moveTime); //Move for this amount of time
        //Serial.println("Moving");
        digitalWrite(enable, LOW); //Enabling the motor
          //delay(50);
      }

//If the microswitch is not pressed the motor shaft will move 
//counterclockwise (LEFT)
      else {
          goingTop = true;
          digitalWrite(enable, HIGH); //Enabling the motor
          digitalWrite(dir, LOW); //Changing movement direction 
            }
      }

delay(5);


}
