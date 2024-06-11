/********************************************************************************************
* 	    	File:  stallguardIsStalled.ino                                                         *
*		   Version:  1.1.0                                                                      *
*         Date:  October 7th, 2023                                                          *
*       Author:  Emil Jacobsen                                                              *
*  Description:  This example demonstrates how Stallguard can be used to detect motor stall,*
*                and automatic stop before any steps is lost.                               *
*                The for-loop applies six different velocities, but only shifts to the next *
*                when a stall is detected.                                                  *
*                Stallguard is very sensitive and provides seamless stall detection when 	  *
*				         tuned for the application. It is dependent on speed, current setting		    *
*				         and load conditions amongst others. The encoder stall detection is 		    *
*				         unaffected by most of these but can be a bit less sensitive.				        *
*                                                                                           *
* For more information, check out the documentation:                                        *
*                       http://ustepper.com/docs/usteppers/html/index.html                  *
*                                                                                           *
*  Note.......:  Place the stepper motor on a solid surface where it cannot harm anyone     *
*                during the test.                                                           *
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
  Info:   This example demonstrates how Stallguard can be used to detect motor stall, and automatic stop before any steps is lost.
          The for-loop applies six different velocities, but only shifts to the next when a stall is detected.  
          For more information, check out the documentation:
          http://ustepper.com/docs/usteppers/html/index.html
*/             

#include <UstepperS32.h>

UstepperS32 stepper;

#define STALLSENSITIVITY 2//sensitivity of the stall detection, between -64 and 63 - high number is less sensitive

uint8_t rpm[6] = {25, 50, 80, 120, 130, 150};

void setup(){
  stepper.setup();						//Initialize uStepper S32
  stepper.checkOrientation(30.0);       //Check orientation of motor connector with +/- 30 microsteps movement  
  Serial.begin(9600);
}


void loop() {

  Serial.println("-- Stallguard test --");

  // Run through all five rpm's
  for( uint8_t i = 0; i < sizeof(rpm); i++ ){
    Serial.print(rpm[i]); Serial.println(" rpm");
    stepper.setRPM(rpm[i]);
    // Enabled stallguard, with threshold set by STALLSENSITIVITY and stop automatic on stall.
    // Hint: If stopOnStall is set to false, you have to use .stop() or setRPM(0) in order to stop the motor on stall. 
    stepper.enableStallguard(STALLSENSITIVITY, true, rpm[i]);

    // Wait for stall to be detected - print stall value as we wait
    while( !stepper.isStalled() )
    {
      // Use the Serial Plotter to effectively see this value. Must be above 0 when running and stall will make it drop. 
      // If 0 when running, increase Sensitivity, if not running decrease Sensitivity
      Serial.println(stepper.driver.getStallValue());
    }
    // Clear stallguard
    stepper.clearStall();
    
    delay(2000);
  }
  
}
