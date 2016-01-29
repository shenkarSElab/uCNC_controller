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
 *
 ********************************************************************************
 *
 *  From the documentation: These are the supported modes for the steppers 
 *                           and light/servo drivers
 *                           
 *	M150: Set Z-Axis mode
 *	---------------------
 *	Defines the mode how the Steppers, the servo and the laser driver are
 *	used by the controller and which codes modify their states
 *	  Mode 0: Stepper 1 is X
 *	          Stepper 2 is Y
 *	          Stepper 3 is Z
 *	          Servo is spindle speed
 *
 *	          
 *	  Mode 1: Stepper 1 is X
 *	          Stepper 2 is Y
 *	          Stepper 3 is Y
 *	          Servo     is Z (down for Z<0)
 *	          Laser     is Z (on for Z<0)
 *	
 *	  Mode 2: Stepper 1 is X
 *	          Stepper 2 is Y
 *	          Stepper 3 is Y
 *  	          Servo     is Z (+90 to -90)
 * 	          Laser     is driven by spindle
 *	          
 *	  Mode 3: Stepper 1 is X
 *	          Stepper 2 is Y
 *	          Stepper 3 is Y
 *	          Servo     is tool
 *  	          Laser     is Z (on for Z<0)
 *	          
 *	  Sample:
 *	    M150 S2   ;Sets mode 2
 *
 *
 */

/* IMPORTANT: All direct motor controls are here, everywhere else
              these functions here are used to keep this code portable 
              
              The functions are grouped into those using the native steps
              values or the measurement (float) values. */

#define DRILL_SPEED 0.1
#define DRILL_DEPTH 10 //mm

int posServo;

void initMotors()
{
  /*Set stepper base speed */
  myStepperX.setSpeed(stepIssueFrequency_X);
  myStepperY.setSpeed(stepIssueFrequency_Y);
  myStepperZ.setSpeed(stepIssueFrequency_Z);
  
  /*Set steppers slack (if any)*/
  myStepperX.setSlack(stepDriveSlack_X);
  myStepperY.setSlack(stepDriveSlack_Y);
  myStepperZ.setSlack(stepDriveSlack_Z);

  myServo.attach(SERVO_PIN);
  myServo.write(servoPosMax);
  posServo = servoPosMax;
}

void homeXYZ()
{
  int i=0;
  
  if(pos_known) {
    movePosXYZ (0, 0, 0, 0);
    return;
  }
#ifdef DO_RESET  
  if (have_endswitch) {
    /* if the end-switch is on go forward a bit */
    if (analogRead(ENDSW_PIN) > 200)
      moveX(100*sgn(stepsPerMillimeter_X)); 
    if (analogRead(ENDSW_PIN) > 200)
      moveY(100*sgn(stepsPerMillimeter_Y)); 
    if (motorMode==0 && analogRead(ENDSW_PIN) > 200)
      moveZ(100*sgn(stepsPerMillimeter_Z)); 
    
    if (analogRead(ENDSW_PIN) > 200) // fail, switch is still on
      return;    
    
    while(i++<2000 && analogRead(ENDSW_PIN) < 200)
      moveX(-1*sgn(stepsPerMillimeter_X));
    i=0;
    while(i++<20 && analogRead(ENDSW_PIN) > 200)
      moveX(1*sgn(stepsPerMillimeter_X));

    i=0;
    while(i++<2000 && analogRead(ENDSW_PIN) < 200)
      moveY(-1*sgn(stepsPerMillimeter_Y));
    i=0;
    while(i++<20 && analogRead(ENDSW_PIN) > 200)
      moveY(1*sgn(stepsPerMillimeter_Y));

    if (motorMode == 0) {
      i=0;
      while(i++<2000 && analogRead(ENDSW_PIN) < 200)
        moveZ(-1*sgn(stepsPerMillimeter_Z));
      i=0;
      while(i++<20 && analogRead(ENDSW_PIN) > 200)
        moveZ(1*sgn(stepsPerMillimeter_Z));
    }
    
  } else {
    myStepper1.step(RESET_TRAVEL_X);
    myStepper2.step(RESET_TRAVEL_Y);
    if (motorMode == 0) {
      myStepper3.step(RESET_TRAVEL_Z);
      delay(500);
      myStepper3.step(RESET_PRELOAD_Z);
    }
    myStepper2.step(RESET_PRELOAD_Y);
    myStepper1.step(RESET_PRELOAD_X);
  }
#endif  
  resetPosXYZ();
}

int moveX(posval_t dX, char *px, char *py, char *pz)
{
  int i;
  X = X + dX;

  *px += tristate(dX);
  return stepIssueFrequency_X;
}

int moveY(posval_t dY, char *px, char *py, char *pz)
{
  Y = Y + dY;
  
  switch(motorMode) {
  case 0:
    *py += tristate(dY);
    break;
  case 1:
  case 2:
  case 3:
    *py += tristate(dY);  // Stepper 2 and 3 are Y
    *pz += tristate(dY);
    break;
  }  
  return (stepIssueFrequency_Y);
}

void resetXYZ()
{
  Y = 0;
  X = 0;
  Z = 0;
}

void updateServo(int servoPos)
{
  if (servoPos>servoPosMax)
    servoPos = servoPosMax;
  if (servoPos<servoPosMin)
    servoPos = servoPosMin;
  
  myServo.write(servoPos);
  posServo = servoPos;
}

void servoZ()
{
  int servoPos = ((float)posZ*servoPosZfactor) + 90;
  updateServo(servoPos);
}

int moveZ(posval_t dZ, char *px, char *py, char *pz)
{
  Z = Z + dZ;  

  switch(motorMode) {
  case 0:
    *pz += tristate(dZ);
    break;
  case 1:
    int pServo;
    pServo = (Z < Z_TRIP_VAL) ? servoPosMin : servoPosMax;
    digitalWrite(LED_PIN,(Z < Z_TRIP_VAL) ? HIGH : LOW);
    if ( pServo != posServo ) {
      updateServo(pServo);
      delay(100);
    }
    return 0;
    break;
  case 2:
    updateServo(servoPosMin + (servoPosZfactor*posZ));
    delay(10);
    break;
  case 3:
    digitalWrite(LED_PIN,(posZ > 0) ? HIGH : LOW);
    break;
  }
  return (stepIssueFrequency_Z);
}

void powerdown()
{
  myStepperX.powerdown();
  myStepperY.powerdown();
  myStepperZ.powerdown();
}

void updateMotorCodes()
{
  digitalWrite(GP1_PIN,(coolant1 == 1) ? HIGH : LOW);
  digitalWrite(GP2_PIN,(coolant2 == 1) ? HIGH : LOW);

#ifdef DIR_PIN
  digitalWrite(DIR_PIN,(spindle > 1) ? HIGH : LOW);
#endif
  
  switch(motorMode) {
  case 0:
    myServo.write(spindleSpeed);
    digitalWrite(LED_PIN,(spindle > 0) ? HIGH : LOW);
    break;
  case 2:
    digitalWrite(LED_PIN,(spindle > 0) ? HIGH : LOW);
    break;
  }
}

void updateToolCodes()
{
  if (motorMode == 3) {
    updateServo(servoPosMin + (tool * servoToolInc));
  }
}

/* No direct IO below this line */

void _moveToXYZ(int pX, int pY, int pZ, int accelX, int accelY, int accelZ)
{ 
  int fx,fy,fz;
  char px,py,pz;   // The frequencies for each stepper

  fx=0; //motor frequencies
  fy=0;
  fz=0;

  px=0; //position increments
  py=0;
  pz=0;

  if (pX - X)
    fx = moveX(pX - X, &px, &py, &pz);
  if (pY - Y)
    fy = moveY(pY - Y, &px, &py, &pz);
  if (pZ - Z)
    fz = moveZ(pZ - Z, &px, &py, &pz);

  /* in theory px,py,pz should never be larger than
   * 2 or smaller than -2. Therefore there should never
   * be more than 2 move schedules here. */
  
  if (fx) {
    myStepperX.chk(fx+accelX,0);
  }
  if (fy) {
    myStepperY.chk(fy+accelY,0);
  }
  if (fz) {
    myStepperZ.chk(fz+accelZ,0);
  }

  if (px) {
    myStepperX.update(tristate(px));
  }
  if (py) {
    myStepperY.update(tristate(py));
  }
  if (pz) {
    myStepperZ.update(tristate(pz));
  } 
}

int accelerate( posval_t pos,posval_t distance, int acceleration, int limit )
{
  unsigned long accval;
  
  if ( pos < distance/2 ) {
    // accelerate 
    accval = pos * acceleration;
    if (accval < limit)
      return (int)accval;
  }
  else {
    // brake
    accval = (distance-pos) * acceleration;
    if (accval < limit)
      return (int)accval;
  }
  
  return limit;
}

void moveToXYZ(posval_t pX, posval_t pY, posval_t pZ, int accelX, int accelY, int accelZ)
{ 
  posval_t absX = abs(pX - X);
  posval_t absY = abs(pY - Y);
  posval_t absZ = abs(pZ - Z);
  
  posval_t deltaX = absX;
  posval_t deltaY = absY;
  posval_t deltaZ = absZ;

  int mX,mY,mZ;
  
  while (absX || absY || absZ)
  {
    mX = 0;
    mY = 0;
    mZ = 0;
    
    if (absX) {
      mX = tristate(pX - X);
      absX--;
      accelX=accelerate(deltaX-absX, deltaX, stepIssueFreqRamp_X, stepIssueFrequencyRampMax_X);
    }
    if (absY) {
      mY = tristate(pY - Y);
      absY--;
      accelY=accelerate(deltaY-absY, deltaY, stepIssueFreqRamp_Y, stepIssueFrequencyRampMax_Y);
    }
    if (absZ) {
      mZ = tristate(pZ - Z);
      absZ--;
      accelZ=accelerate(deltaZ-absZ, deltaZ, stepIssueFreqRamp_Z, stepIssueFrequencyRampMax_Z);
    }
    _moveToXYZ(X + mX, Y + mY, Z + mZ, accelX, accelY, accelZ);
  }
}

void wait(unsigned long steptime)
{
  if (!steptime)
    return;
    
  while (micros() < steptime ) {
    delayMicroseconds(5);
  }
  
  return;
}

void lineXYZ(posval_t x2, posval_t y2, posval_t z2, float feedrate)
{
  posval_t n, deltax, deltay, deltaz, sgndeltax, sgndeltay, sgndeltaz, deltaxabs, deltayabs, deltazabs, x, y, z, drawx, drawy, drawz;

  deltax = x2 - X;
  deltay = y2 - Y;
  deltaz = z2 - Z;
  deltaxabs = abs(deltax);
  deltayabs = abs(deltay);
  deltazabs = abs(deltaz);
  sgndeltax = sgn(deltax);
  sgndeltay = sgn(deltay);
  sgndeltaz = sgn(deltaz);
  x = deltaxabs >> 1;
  y = deltayabs >> 1;
  z = deltazabs >> 1;
  drawx = X;
  drawy = Y;
  drawz = Z;
  
  int accel=0;
  
  unsigned long stepdelay = 0;
  unsigned long laststep = 0;
  
  int delay_X=60000000/(fabs(stepsPerMillimeter_X) * feedrate);
  int delay_Y=60000000/(fabs(stepsPerMillimeter_Y) * feedrate);
  int delay_Z=60000000/(fabs(stepsPerMillimeter_Z) * feedrate);
    
  moveToXYZ(drawx, drawy, drawz, 0, 0, 0);
  laststep=micros();
  
  // dX is biggest
  if(deltaxabs >= deltayabs && deltaxabs >= deltazabs){
    for(n = 0; n < deltaxabs; n++){
      stepdelay = delay_X;

      y += deltayabs;
      if(y >= deltaxabs){
        y -= deltaxabs;
        drawy += sgndeltay;
        stepdelay += delay_Y;
      }
      z += deltazabs;
      if(z >= deltaxabs){
        z -= deltaxabs;
        drawz += sgndeltaz;
        stepdelay += delay_Z;
      }

      drawx += sgndeltax;
      
      accel = accelerate(n,deltaxabs, stepIssueFreqRamp_X, stepIssueFrequencyRampMax_X);

      wait(laststep+stepdelay);
      laststep=micros();
      _moveToXYZ(drawx, drawy, drawz, accel, (deltaxabs == deltayabs ? accel : 0), 0);
    }
    return;
  }
  // dY is biggest
  if(deltayabs >= deltaxabs && deltayabs >= deltazabs){
    for(n = 0; n < deltayabs; n++){
      stepdelay = delay_Y;

      x += deltaxabs;
      if(x >= deltayabs){
        x -= deltayabs;
        drawx += sgndeltax;
        stepdelay += delay_X;
      }
      z += deltazabs;
      if(z >= deltayabs){
        z -= deltayabs;
        drawz += sgndeltaz;
        stepdelay += delay_Z;
      }
      drawy += sgndeltay;

      accel = accelerate(n,deltayabs, stepIssueFreqRamp_Y, stepIssueFrequencyRampMax_Y);

      wait(laststep+stepdelay);
      laststep=micros();
      _moveToXYZ(drawx, drawy, drawz, 0, accel, 0);
    }
    return;
  }
  // dZ is biggest
  if(deltazabs >= deltaxabs && deltazabs >= deltayabs){
    for(n = 0; n < deltazabs; n++){
      stepdelay = delay_Z;

      x += deltaxabs;
      if(x >= deltazabs){
        x -= deltazabs;
        drawx += sgndeltax;
        stepdelay += delay_X;
      }
      y += deltayabs;
      if(y >= deltazabs){
        y -= deltazabs;
        drawy += sgndeltay;
        stepdelay += delay_Y;
      }
      drawz += sgndeltaz;

      accel = accelerate(n,deltazabs, stepIssueFreqRamp_Z, stepIssueFrequencyRampMax_Z);

      wait(laststep+stepdelay);
      laststep=micros();
      _moveToXYZ(drawx, drawy, drawz, 0, 0, accel);
    }
    return;
  }
}

/* No direct use of step coordinates below this line */

posval_t convertPosX(float pos)
{ 
  return (posval_t)(pos*conversionFactor*stepsPerMillimeter_X);
}

posval_t convertPosY(float pos)
{ 
  return (posval_t)(pos*conversionFactor*stepsPerMillimeter_Y);
}

posval_t convertPosZ(float pos)
{ 
  return (posval_t)(pos*conversionFactor*stepsPerMillimeter_Z);
}

void linePos(float x2, float y2, float z2, float feedrate)
{
  lineXYZ(convertPosX(x2),convertPosY(y2),convertPosZ(z2), feedrate);
  posX=x2;
  posY=y2;
  posZ=z2;
}

void jumpPos(float x2, float y2, float z2)
{
  moveToXYZ(convertPosX(x2),convertPosY(y2),convertPosZ(z2), 0, 0, 0);
  posX=x2;
  posY=y2;
  posZ=z2;
}

void movePosXYZ (float x2, float y2, float z2, float feedrate )
{
  linePos(x2, y2, z2, feedrate);
}

void jumpPosXYZ (float x2, float y2, float z2)
{
  jumpPos(x2, y2, z2);
}

void resetPosXYZ()
{
  resetXYZ();
  posX = 0.0;
  posY = 0.0;
  posZ = 0.0;
  pos_known=true; 
}
