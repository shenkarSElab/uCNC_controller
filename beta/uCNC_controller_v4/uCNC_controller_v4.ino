
#include <Servo.h>
#include "uCNC.h"

/* Version of this progam */
float uCNC_Version = 1.99;

/* Development functions*/
//#define BUILTIN 1


/* Define this if you want the machine to perform
   a reset during startup, else, the first set_home
   or go_home will initialize it.*/
//#define DO_RESET 1

/* Conversion factor of steps per millimeter 
   use the slack/backlash variable, if you have
   gear or drive issues.
   The RampMax value is added on top of the step
   freqency, which is the start step rate in HZ.
   The ramp rise is set with the Ramp value */
float stepsPerMillimeter_X = 80;
int   stepIssueFrequency_X  = 400;
int   stepIssueFreqRamp_X = 10;
int   stepIssueFrequencyRampMax_X  = 200;
int   stepDriveSlack_X = 0;

float stepsPerMillimeter_Y = 80;
int   stepIssueFrequency_Y  = 400;
int   stepIssueFreqRamp_Y = 10;
int   stepIssueFrequencyRampMax_Y  = 200;
int   stepDriveSlack_Y = 0;

float stepsPerMillimeter_Z = 100;
int   stepIssueFrequency_Z  = 400;
int   stepIssueFreqRamp_Z = 5;
int   stepIssueFrequencyRampMax_Z  = 100;
int   stepDriveSlack_Z = 0;

/* Unit conversion factor */
float conversionFactor = 1;  // 1 for mm 25.4 for inches

/* Stepper library initialization 
   README:
   Depending on the type of control board you
   are using, there are big diffenences here.
   Refer to the code in uCNC_stepper.cpp to understand
   the code, and and make the right choices here. */

//DIR, STEP ,EN
Stepper myStepperY(3,4,2);
Stepper myStepperX(12, 11,10);            
Stepper myStepperZ(7,4,8);            
#define SERVO_PIN  9   //Servo output

/* General purpose outputs */
#define LED_PIN    A5   //LED/LASER output
#define DIR_PIN    13   //LED/LASER output
#define GP1_PIN    A3   //General pupose (coolant 1) output
#define GP2_PIN    A4   //General pupose (coolant 2) output
#define GP3_PIN    A2   //General pupose
#define ENDSW_PIN  A0   //Start/End Switch input


/* Servo functions and limits */
Servo myServo;

int servoPosMax=83;
int servoPosMin=70;
int servoToolInc=10;
float servoPosZfactor=1.0;

/* Mode selector for the motors (see documentation) */
int   motorMode = 1;

/* Important value for laser and servo mode defines the height
   at which the laser is turned on */
#define Z_TRIP_VAL 0.0001

/* X,Y,Z in absolute steps position */
posval_t X = 0;
posval_t Y = 0;
posval_t Z = 0;

/* Set to false to have the first G28(home) reset the axis */
boolean pos_known = false;

/* Set to true if there is are end-switches on A0 */
boolean have_endswitch = false;

/* For non-endswitch reset */
#define RESET_TRAVEL_X -1500
#define RESET_TRAVEL_Y -1500
#define RESET_TRAVEL_Z -1500
#define RESET_PRELOAD_X 20
#define RESET_PRELOAD_Y 400
#define RESET_PRELOAD_Z 0

/* X,Y,Z in measurement value*/
float posX = 0.0;
float posY = 0.0;
float posZ = 0.0;

/* Tools and Feeds and Coolants */
int tool     = 0;
int spindle  = 0;
int coolant1 = 0;
int coolant2 = 0;

/* Spindle speed (M3 parameter)*/
int spindleSpeed = 0;

#define COMMAND_SIZE 128
uint8_t command_line[COMMAND_SIZE];
uint8_t sin_count=0;
uint16_t no_data = 0;
uint8_t asleep = 0;

void setup() {
  Serial.begin(9600);
  
  // LED (Laser output)
  pinMode(LED_PIN, OUTPUT);
  pinMode(GP1_PIN, OUTPUT);
  pinMode(GP2_PIN, OUTPUT);
  pinMode(GP3_PIN, OUTPUT);
  
  /* Init the steppers and servo */
  initMotors();
}

void clear_command_string() {
  for (int i=0; i<COMMAND_SIZE; i++) 
    command_line[i] = 0;
  sin_count = 0;
}

void loop() {
  uint8_t c;
  
  Serial.println("uCNC_controller_v4 (compatible: Grbl v0.81, Repetier 0.80, E3D v1.00)");
  Serial.println("ready");

  while (true) {
    //read in characters if we got them.
    if (Serial.available() > 0)   {
      c = (uint8_t)Serial.read();
      no_data = 0;
      asleep = 0;
      command_line[sin_count++] = c;
    }
    else {
      no_data++;
      delayMicroseconds(150);
    }
  
    if (sin_count && (c == '\n' || no_data > 100)) {
      command_line[sin_count] = 0;
      process_command(command_line);
      clear_command_string(); 
    }
  
    if (no_data == 60000)  {
      if (!asleep) {
        powerdown();
        asleep=1;
      }
    }
  }
}

/*
 * This file is part of uCNC_controller.
 *
 * Copyright (C) 2014  D.Herrendoerfer
 *
 *   uCNC_controller is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   uCNC_controller is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with uCNC_controller.  If not, see <http://www.gnu.org/licenses/>.
 */

