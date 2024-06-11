/********************************************************************************************
* 	    	File:  continous.ino                                                              *
*		    Version:    2.3.0                                          						    *
*      	Date: 		October 7th, 2023 	                                    			*
*       Author:  Thomas Hørring Olsen                                                       *
*  Description:  Continous Example Sketch!                                                  *
*                This example demonstrates how the library can be used to make the motorrun *
*                continously, in both directions and making it stop an ongoing movement.    *
*                                                                                           *
* For more information, check out the documentation:                                        *
*    http://ustepper.com/docs/usteppers/html/index.html                                     *
*                                                                                           *
*                                                                                           *
*********************************************************************************************
*	(C) 2023                                                                                  *
*                                                                                           *
*	uStepper ApS                                                                              *
*	www.ustepper.com                                                                          *
*	administration@ustepper.com                                                               *
*                                                                                           *
*	The code contained in this file is released under the following open source license:      *
*                                                                                           *
*			Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International               *
*                                                                                           *
* 	The code in this file is provided without warranty of any kind - use at own risk!       *
* 	neither uStepper ApS nor the author, can be held responsible for any damage             *
* 	caused by the use of the code contained in this file !                                  *
*                                                                                           *
*                                                                                           *
********************************************************************************************/

/*
*      Continous Example Sketch!
*
* This example demonstrates how the library can be used to make the motorrun continously,
* in both directions and making it stop an ongoing movement.
* For more information, check out the documentation:
* http://ustepper.com/docs/usteppers/html/index.html
*/
#include <UstepperS32.h>

UstepperS32 stepper;

void setup(){
  stepper.setup();					 //Initialize uStepper S32
  stepper.checkOrientation(30.0);    //Check orientation of motor connector with +/- 30 microsteps movement
  Serial.begin(9600);
  stepper.setRPM(-100);				 //Set speed to -100
}

void loop() {
  char cmd;

  // put your main code here, to run repeatedly:
  while(!Serial.available());
  Serial.println("ACK!");
  cmd = Serial.read();
  if(cmd == '1')                      //Run continous clockwise
  {
    stepper.setRPM(-100);
  }
  
  else if(cmd == '2')                 //Run continous counter clockwise
  {
    stepper.setRPM(100);
  }
  
  else if(cmd == '3')                 //Stop without deceleration and block motor
  {
    stepper.stop();
  }
}